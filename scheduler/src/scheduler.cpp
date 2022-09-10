#include <filesystem>
#include <string>
#include <system_error>
#include <unordered_set>

#include "constants.h"
#include "error.h"
#include "json.h"
#include "operations.h"
#include "result.h"
#include "scheduler.h"
#include "uuid.h"

namespace fs = std::filesystem;

bool IsFilenameValid(const std::string &filename) {
    return !filename.empty() && filename != "." && filename != ".." &&
           filename.find('/') == std::string::npos;
}

bool IsAncestor(const fs::path &ancestor, const fs::path &path) {
    fs::path p1 = fs::canonical(ancestor), p2 = fs::canonical(path);
    return std::mismatch(p1.begin(), p1.end(), p2.begin()).first == p1.end();
}

void GraphState::Validate() const {
    for (const auto &block : blocks) {
        std::unordered_set<std::string> filenames;
        for (const auto &bind : block.binds) {
            filenames.insert(bind.inside);
        }
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
        if (filenames.size() != block.binds.size() + block.inputs.size() + block.outputs.size()) {
            throw ValidationError(errors::kDuplicatedFilename);
        }
    }
    for (const auto &[start_block_id, start_output_id, end_block_id, end_input_id] : connections) {
        if (start_block_id < 0 || start_block_id >= blocks.size() ||
            start_output_id >= blocks[start_block_id].outputs.size() || end_block_id < 0 ||
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
    Load(*static_cast<Graph *>(this), document);
    Validate();
    blocks_state_.resize(blocks.size());
    go_.resize(blocks.size());
    for (const auto &connection : connections) {
        go_[connection.start_block_id].push_back(connection);
    }
}

void GraphState::Run() {
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

void GraphState::Stop() {
    // TODO
    throw APIError(errors::kNotImplemented);
}

void GraphState::RunBlock(size_t block_id, RunnerWebSocket *ws) {
    ws->getUserData()->graph_ptr = this;
    ws->getUserData()->block_id = block_id;
    BlockResponse block_response = {.block_id = static_cast<int>(block_id),
                                    .state = states::kRunning};
    try {
        PrepareContainer(block_id);
        SendTask(block_id, ws);
        SendToAllClients(StringifyJSON(Dump(block_response)));
    } catch (const fs::filesystem_error &error) {
        block_response.state = states::kComplete;
        block_response.error = errors::kRuntimeErrorPrefix + error.what();
        SendToAllClients(StringifyJSON(Dump(block_response)));
        ClearBlockState(block_id);
        partition_ptr->AddRunner(ws);
        DequeueBlock();
        UpdateBlocksProcessing();
    }
}

void GraphState::OnResult(RunnerWebSocket *ws, std::string_view message) {
    size_t block_id = ws->getUserData()->block_id;
    RunResponse run_response;
    Load(run_response, ParseJSON(std::string(message)));
    BlockResponse block_response = {.block_id = static_cast<int>(block_id),
                                    .state = states::kComplete,
                                    .error = run_response.error,
                                    .result = run_response.result};
    if (run_response.result.has_value() && run_response.result->exited &&
        run_response.result->exit_code == 0) {
        try {
            for (const auto &connection : go_[block_id]) {
                if (TransferFile(connection) && IsBlockReady(connection.end_block_id)) {
                    EnqueueBlock(connection.end_block_id);
                }
            }
        } catch (const fs::filesystem_error &error) {
            block_response.error = errors::kRuntimeErrorPrefix + error.what();
        }
    }
    SendToAllClients(StringifyJSON(Dump(block_response)));
    ClearBlockState(block_id);
    partition_ptr->AddRunner(ws);
    DequeueBlock();
    UpdateBlocksProcessing();
}

void GraphState::EnqueueBlock(size_t block_id) {
    blocks_ready_.push(block_id);
}

void GraphState::DequeueBlock() {
    --cnt_blocks_processing_;
}

void GraphState::UpdateBlocksProcessing() {
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

void GraphState::PrepareContainer(size_t block_id) {
    std::string container = GetContainer(block_id);
    fs::path container_path = fs::path(SANDBOX_DIR) / container;
    if (!fs::exists(container_path)) {
        fs::create_directories(container_path);
        fs::permissions(container_path, fs::perms::all, fs::perm_options::add);
    }
    for (const auto &bind : blocks[block_id].binds) {
        fs::path bind_inside_path = container_path / bind.inside;
        fs::path bind_outside_path = fs::path(USER_DIR) / bind.outside;
        if (!IsAncestor(fs::path(USER_DIR), bind_outside_path)) {
            throw fs::filesystem_error("cannot bind", bind_outside_path,
                                       std::make_error_code(std::errc::permission_denied));
        }
        fs::copy(bind_outside_path, bind_inside_path,
                 fs::copy_options::create_hard_links | fs::copy_options::recursive |
                     fs::copy_options::skip_symlinks);
    }
}

bool GraphState::TransferFile(const Connection &connection) {
    const auto &[start_block_id, start_output_id, end_block_id, end_input_id] = connection;
    std::string start_container = GetContainer(start_block_id);
    std::string start_output_name = blocks[start_block_id].outputs[start_output_id].name;
    std::string end_container = GetContainer(end_block_id);
    std::string end_input_name = blocks[end_block_id].inputs[end_input_id].name;
    fs::path end_container_path = fs::path(SANDBOX_DIR) / end_container;
    if (!fs::exists(end_container_path)) {
        fs::create_directories(end_container_path);
        fs::permissions(end_container_path, fs::perms::all, fs::perm_options::add);
    }
    fs::path start_output_path = fs::path(SANDBOX_DIR) / start_container / start_output_name;
    fs::path end_input_path = fs::path(SANDBOX_DIR) / end_container / end_input_name;
    if (!fs::exists(start_output_path) || fs::exists(end_input_path)) {
        return false;
    }
    fs::copy(start_output_path, end_input_path,
             fs::copy_options::create_hard_links | fs::copy_options::recursive |
                 fs::copy_options::skip_symlinks);
    ++blocks_state_[end_block_id].cnt_inputs_ready;
    return true;
}

void GraphState::SendTask(size_t block_id, RunnerWebSocket *ws) {
    RunRequest run_request = {.container = GetContainer(block_id), .task = blocks[block_id].task};
    ws->send(StringifyJSON(Dump(run_request)));
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

std::string GraphState::GetContainer(size_t block_id) const {
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
