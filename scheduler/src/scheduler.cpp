#include <filesystem>
#include <string>
#include <unordered_set>

#include "constants.h"
#include "error.h"
#include "json.h"
#include "scheduler.h"
#include "serialization/all.h"
#include "structures/all.h"
#include "uuid.h"

namespace fs = std::filesystem;

void WorkflowState::Init(const rapidjson::Document &document) {
    Deserialize(static_cast<Workflow &>(*this), document);
    blocks_state_.resize(blocks.size());
    go_.resize(blocks.size());
    for (const auto &block : blocks) {
        std::unordered_set<std::string> locations;
        locations.insert(block.inputs.begin(), block.inputs.end());
        locations.insert(block.outputs.begin(), block.outputs.end());
        for (const auto &bind : block.binds) {
            locations.insert(bind.inside);
        }
        if (locations.size() != block.inputs.size() + block.outputs.size() + block.binds.size()) {
            throw ValidationError(errors::kDuplicatedLocation);
        }
    }
    for (const auto &connection : connections) {
        if (connection.source_block_id >= blocks.size() ||
            connection.source_output_id >= blocks[connection.source_block_id].outputs.size() ||
            connection.target_block_id >= blocks.size() ||
            connection.target_input_id >= blocks[connection.target_block_id].inputs.size()) {
            throw ValidationError(errors::kInvalidConnection);
        }
        if (connection.source_block_id == connection.target_block_id) {
            throw ValidationError(errors::kLoopsNotSupported);
        }
        go_[connection.source_block_id].push_back(connection);
    }
    for (size_t block_id = 0; block_id < blocks.size(); ++block_id) {
        blocks_state_[block_id].input_sources.resize(blocks[block_id].inputs.size());
    }
}

void WorkflowState::Run() {
    if (is_running_) {
        throw APIError(errors::kAlreadyRunning);
    }
    is_running_ = true;
    for (size_t block_id = 0; block_id < blocks.size(); ++block_id) {
        if (IsBlockReady(block_id)) {
            EnqueueBlock(block_id);
        }
    }
    UpdateBlocksProcessing();
}

void WorkflowState::Stop() {
    // TODO
    throw APIError(errors::kNotImplemented);
}

void WorkflowState::RunBlock(size_t block_id, RunnerWebSocket *ws) {
    ws->getUserData()->workflow_ptr = this;
    ws->getUserData()->block_id = block_id;
    try {
        PrepareRun(block_id);
        SendRunRequest(block_id, ws);
        BlockResponse response = {.block_id = block_id, .state = states::kRunning};
        SendToAllClients(StringifyJSON(Serialize(response)));
    } catch (const fs::filesystem_error &error) {
        RunResponse response = {.error = errors::kRuntimeErrorPrefix + error.what()};
        OnStatus(ws, StringifyJSON(Serialize(response)));
    }
}

void WorkflowState::OnStatus(RunnerWebSocket *ws, std::string_view message) {
    size_t block_id = ws->getUserData()->block_id;
    RunResponse run_response;
    Deserialize(run_response, ParseJSON(std::string(message)));
    if (run_response.status.has_value() && run_response.status->exited &&
        run_response.status->exit_code == 0) {
        for (const auto &connection : go_[block_id]) {
            if (ProcessConnection(connection) && IsBlockReady(connection.target_block_id)) {
                EnqueueBlock(connection.target_block_id);
            }
        }
    }
    FinalizeRun(block_id);
    BlockResponse block_response = {.block_id = block_id,
                                    .state = states::kComplete,
                                    .error = run_response.error,
                                    .status = run_response.status};
    SendToAllClients(StringifyJSON(Serialize(block_response)));
    partition_ptr->AddRunner(ws);
    DequeueBlock();
    UpdateBlocksProcessing();
}

void WorkflowState::EnqueueBlock(size_t block_id) {
    blocks_ready_.push(block_id);
}

void WorkflowState::DequeueBlock() {
    --cnt_blocks_processing_;
}

void WorkflowState::UpdateBlocksProcessing() {
    while (!blocks_ready_.empty() && cnt_blocks_processing_ < meta.max_runners) {
        size_t block_id = blocks_ready_.front();
        blocks_ready_.pop();
        ++cnt_blocks_processing_;
        partition_ptr->EnqueueBlock(this, block_id);
    }
    if (is_running_ && cnt_blocks_processing_ == 0 && blocks_ready_.empty()) {
        is_running_ = false;
        SendToAllClients(states::kComplete);
    }
}

bool WorkflowState::ProcessConnection(const Connection &connection) {
    const auto &[source_block_id, source_output_id, target_block_id, target_input_id] = connection;
    fs::path source_output_path = fs::path("containers") / GetContainerId(source_block_id) /
                                  blocks[source_block_id].outputs[source_output_id];
    if (!fs::exists(fs::path(paths::kVarDir) / source_output_path) ||
        blocks_state_[target_block_id].input_sources[target_input_id]) {
        return false;
    }
    ++blocks_state_[target_block_id].cnt_inputs_ready;
    blocks_state_[target_block_id].input_sources[target_input_id].emplace(source_output_path);
    return true;
}

bool WorkflowState::IsBlockReady(size_t block_id) const {
    return blocks_state_[block_id].cnt_inputs_ready == blocks[block_id].inputs.size();
}

void WorkflowState::PrepareRun(size_t block_id) {
    fs::path container_path = fs::path(paths::kVarDir) / "containers" / GetContainerId(block_id);
    fs::create_directories(container_path);
    fs::permissions(container_path, fs::perms::all, fs::perm_options::add);
}

void WorkflowState::FinalizeRun(size_t block_id) {
    blocks_state_[block_id].cnt_inputs_ready = 0;
    ++blocks_state_[block_id].cnt_runs;
    blocks_state_[block_id].input_sources.assign(blocks[block_id].inputs.size(), {});
}

void WorkflowState::SendRunRequest(size_t block_id, RunnerWebSocket *ws) {
    fs::path container_path = fs::path("containers") / GetContainerId(block_id);
    std::vector<Bind> binds = {
        {.inside = ".", .outside = container_path.string(), .readonly = false}};
    for (size_t input_id = 0; input_id < blocks[block_id].inputs.size(); ++input_id) {
        binds.push_back({.inside = blocks[block_id].inputs[input_id],
                         .outside = blocks_state_[block_id].input_sources[input_id].value(),
                         .readonly = true});
    }
    for (const auto &bind : blocks[block_id].binds) {
        binds.push_back({.inside = bind.inside,
                         .outside = (fs::path("user") / bind.outside).string(),
                         .readonly = bind.readonly});
    }
    RunRequest request = {.binds = std::move(binds),
                          .argv = blocks[block_id].argv,
                          .env = blocks[block_id].env,
                          .constraints = blocks[block_id].constraints};
    ws->send(StringifyJSON(Serialize(request)));
}

void WorkflowState::AddClient(ClientWebSocket *ws) {
    clients_.insert(ws);
}

void WorkflowState::RemoveClient(ClientWebSocket *ws) {
    clients_.erase(ws);
}

void WorkflowState::SendToAllClients(std::string_view message) {
    for (ClientWebSocket *ws : clients_) {
        ws->send(message);
    }
}

std::string WorkflowState::GetContainerId(size_t block_id) const {
    return workflow_id + "_" + std::to_string(block_id) + "_" +
           std::to_string(blocks_state_[block_id].cnt_runs);
}

void Partition::AddRunner(RunnerWebSocket *ws) {
    if (blocks_waiting_.empty()) {
        runners_waiting_.insert(ws);
    } else {
        auto [workflow_ptr, block_id] = blocks_waiting_.front();
        blocks_waiting_.pop();
        workflow_ptr->RunBlock(block_id, ws);
    }
}

void Partition::RemoveRunner(RunnerWebSocket *ws) {
    runners_waiting_.erase(ws);
}

void Partition::EnqueueBlock(WorkflowState *workflow_ptr, size_t block_id) {
    if (runners_waiting_.empty()) {
        blocks_waiting_.emplace(workflow_ptr, block_id);
    } else {
        RunnerWebSocket *ws = runners_waiting_.extract(runners_waiting_.begin()).value();
        workflow_ptr->RunBlock(block_id, ws);
    }
}

void Scheduler::JoinRunner(RunnerWebSocket *ws) {
    std::string partition = ws->getUserData()->partition;
    groups_[partition].AddRunner(ws);
}

void Scheduler::LeaveRunner(RunnerWebSocket *ws) {
    std::string partition = ws->getUserData()->partition;
    groups_[partition].RemoveRunner(ws);
}

void Scheduler::JoinClient(ClientWebSocket *ws) {
    ws->getUserData()->workflow_ptr->AddClient(ws);
}

void Scheduler::LeaveClient(ClientWebSocket *ws) {
    ws->getUserData()->workflow_ptr->RemoveClient(ws);
}

std::string Scheduler::AddWorkflow(const rapidjson::Document &document) {
    std::string workflow_id = GenerateUuid();
    WorkflowState workflow_state;
    workflow_state.Init(document);
    workflow_state.workflow_id = workflow_id;
    workflow_state.partition_ptr = &groups_[workflow_state.meta.partition];
    workflows_[workflow_id] = std::move(workflow_state);
    return workflow_id;
}

WorkflowState *Scheduler::FindWorkflow(const std::string &workflow_id) {
    auto iter = workflows_.find(workflow_id);
    if (iter == workflows_.end()) {
        return nullptr;
    } else {
        return &iter->second;
    }
}
