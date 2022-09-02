#include <filesystem>
#include <string>
#include <unordered_set>

#include "constants.h"
#include "error.h"
#include "json.h"
#include "run_request.h"
#include "run_response.h"
#include "scheduler.h"
#include "uuid.h"

namespace fs = std::filesystem;

bool IsFilenameValid(const std::string &filename) {
    return !filename.empty() && filename.find('/') == std::string::npos && filename[0] != '.';
}

void GraphState::Validate() const {
    for (const auto &block : blocks) {
        std::unordered_set<std::string> filenames;
        for (const auto &input : block.inputs) {
            filenames.insert(input.name);
        }
        for (const auto &output : block.outputs) {
            filenames.insert(output.name);
        }
        for (const auto &filename : filenames) {
            if (!IsFilenameValid(filename)) {
                throw ValidationError(errors::kInvalidFilename);
            }
        }
        if (filenames.size() != block.inputs.size() + block.outputs.size()) {
            throw ValidationError(errors::kDuplicatedFilename);
        }
    }
    for (const auto &[start_block_id, start_output_id, end_block_id, end_input_id] : connections) {
        if (start_block_id >= blocks.size() ||
            start_output_id >= blocks[start_block_id].outputs.size() ||
            end_block_id >= blocks.size() || end_input_id >= blocks[end_block_id].inputs.size()) {
            throw ValidationError(errors::kInvalidConnection);
        }
        if (start_block_id == end_block_id) {
            throw ValidationError(errors::kLoopsNotSupported);
        }
    }
    if (meta.partition.empty()) {
        throw ValidationError(errors::kInvalidPartition);
    }
    if (meta.max_runners < 1) {
        throw ValidationError(errors::kInvalidMaxRunners);
    }
}

void GraphState::Init(const rapidjson::Document &document) {
    Load(document);
    Validate();
    blocks_state_.resize(blocks.size());
    go_.resize(blocks.size());
    for (const auto &connection : connections) {
        go_[connection.start_block_id].push_back(connection);
    }
}

void GraphState::Run() {
    if (IsRunning()) {
        throw APIError(errors::kAlreadyRunning);
    }
    for (size_t block_id = 0; block_id < blocks.size(); ++block_id) {
        if (IsBlockReady(block_id)) {
            EnqueueBlock(block_id);
        }
    }
    if (!IsRunning()) {
        SendToAllClients(signals::kGraphComplete);
    }
}

void GraphState::Stop() {
    // TODO
    throw APIError(errors::kNotImplemented);
}

bool GraphState::IsRunning() const {
    return cnt_blocks_processing_ > 0;
}

void GraphState::RunBlock(size_t block_id, RunnerWebSocket *ws) {
    ws->getUserData()->graph_ptr = this;
    ws->getUserData()->block_id = block_id;
    PrepareBlockContainer(block_id);
    SendTasks(block_id, ws);
}

void GraphState::OnStatus(RunnerWebSocket *ws, std::string_view message) {
    size_t block_id = ws->getUserData()->block_id;
    partition_ptr->AddRunner(ws);
    bool success = ProcessStatus(block_id, message);
    DequeueBlock();
    if (success) {
        try {
            for (const auto &connection : go_[block_id]) {
                if (TransferFile(connection) && IsBlockReady(connection.end_block_id)) {
                    EnqueueBlock(connection.end_block_id);
                }
            }
        } catch (const fs::filesystem_error &error) {
            SendToAllClients(errors::kRuntimeErrorPrefix + error.what());
        }
    }
    ClearBlockState(block_id);
    if (!IsRunning()) {
        SendToAllClients(signals::kGraphComplete);
    }
}

void GraphState::EnqueueBlock(size_t block_id) {
    if (cnt_blocks_processing_ < meta.max_runners) {
        partition_ptr->EnqueueBlock(this, block_id);
        ++cnt_blocks_processing_;
    } else {
        blocks_ready_.push(block_id);
    }
}

void GraphState::DequeueBlock() {
    if (!blocks_ready_.empty()) {
        size_t block_id = blocks_ready_.front();
        blocks_ready_.pop();
        partition_ptr->EnqueueBlock(this, block_id);
    } else {
        --cnt_blocks_processing_;
    }
}

void GraphState::PrepareBlockContainer(size_t block_id) {
    std::string container_name = GetContainerName(block_id);
    if (!fs::exists(fs::path(SANDBOX_DIR) / container_name)) {
        fs::create_directories(fs::path(SANDBOX_DIR) / container_name);
    }
}

bool GraphState::TransferFile(const Connection &connection) {
    const auto &[start_block_id, start_output_id, end_block_id, end_input_id] = connection;
    std::string start_container_name = GetContainerName(start_block_id);
    std::string start_output_name = blocks[start_block_id].outputs[start_output_id].name;
    std::string end_container_name = GetContainerName(end_block_id);
    std::string end_input_name = blocks[end_block_id].inputs[end_input_id].name;
    if (!fs::exists(fs::path(SANDBOX_DIR) / end_container_name)) {
        fs::create_directories(fs::path(SANDBOX_DIR) / end_container_name);
    }
    fs::path start_output_path = fs::path(SANDBOX_DIR) / start_container_name / start_output_name;
    fs::path end_input_path = fs::path(SANDBOX_DIR) / end_container_name / end_input_name;
    if (!fs::exists(start_output_path) || fs::exists(end_input_path)) {
        return false;
    }
    fs::copy(start_output_path, end_input_path,
             fs::copy_options::create_hard_links | fs::copy_options::recursive);
    ++blocks_state_[end_block_id].cnt_inputs_ready;
    return true;
}

void GraphState::SendTasks(size_t block_id, RunnerWebSocket *ws) {
    RunRequest run_request;
    run_request.container = GetContainerName(block_id);
    run_request.tasks = blocks[block_id].tasks;
    ws->send(StringifyJSON(run_request.Dump()));
}

bool GraphState::ProcessStatus(size_t block_id, std::string_view message) {
    RunResponse run_response;
    run_response.Load(ParseJSON(std::string(message)));
    BlockRunResponse block_run_response = run_response;
    block_run_response.block_id = block_id;
    if (run_response.has_error) {
        SendToAllClients(StringifyJSON(block_run_response.Dump()));
        return false;
    }
    bool exited_normally = true;
    for (const auto &status : run_response.statuses) {
        if (!status.exited || status.exit_code != 0) {
            exited_normally = false;
            break;
        }
    }
    SendToAllClients(StringifyJSON(block_run_response.Dump()));
    return exited_normally;
}

bool GraphState::IsBlockReady(size_t block_id) const {
    return blocks_state_[block_id].cnt_inputs_ready == blocks[block_id].inputs.size();
}

void GraphState::ClearBlockState(size_t block_id) {
    blocks_state_[block_id].cnt_inputs_ready = 0;
    ++blocks_state_[block_id].cnt_runs;
}

void GraphState::AddClient(ClientWebSocket *ws) {
    clients_.insert(ws);
}

void GraphState::RemoveClient(ClientWebSocket *ws) {
    clients_.erase(ws);
}

void GraphState::SendToAllClients(std::string_view message) {
    for (ClientWebSocket *ws : clients_) {
        ws->send(message);
    }
}

std::string GraphState::GetContainerName(size_t block_id) const {
    return graph_id + "_" + std::to_string(block_id) + "_" +
           std::to_string(blocks_state_[block_id].cnt_runs);
}

void Partition::AddRunner(RunnerWebSocket *ws) {
    if (blocks_waiting_.empty()) {
        runners_waiting_.insert(ws);
    } else {
        auto [graph_ptr, block_id] = blocks_waiting_.front();
        blocks_waiting_.pop();
        graph_ptr->RunBlock(block_id, ws);
    }
}

void Partition::RemoveRunner(RunnerWebSocket *ws) {
    runners_waiting_.erase(ws);
}

void Partition::EnqueueBlock(GraphState *graph_ptr, size_t block_id) {
    if (runners_waiting_.empty()) {
        blocks_waiting_.emplace(graph_ptr, block_id);
    } else {
        RunnerWebSocket *ws = runners_waiting_.extract(runners_waiting_.begin()).value();
        graph_ptr->RunBlock(block_id, ws);
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
    ws->getUserData()->graph_ptr->AddClient(ws);
}

void Scheduler::LeaveClient(ClientWebSocket *ws) {
    ws->getUserData()->graph_ptr->RemoveClient(ws);
}

std::string Scheduler::AddGraph(const rapidjson::Document &document) {
    std::string graph_id = GenerateUuid();
    GraphState graph_state;
    graph_state.Init(document);
    graph_state.graph_id = graph_id;
    graph_state.partition_ptr = &groups_[graph_state.meta.partition];
    graphs_[graph_id] = std::move(graph_state);
    return graph_id;
}

GraphState *Scheduler::FindGraph(const std::string &graph_id) {
    auto iter = graphs_.find(graph_id);
    if (iter == graphs_.end()) {
        return nullptr;
    } else {
        return &iter->second;
    }
}
