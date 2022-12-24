#include <filesystem>
#include <string>
#include <unistd.h>
#include <unordered_set>

#include "constants.h"
#include "error.h"
#include "json.h"
#include "result.h"
#include "scheduler.h"
#include "serialize.h"
#include "uuid.h"

namespace fs = std::filesystem;

void GraphState::Init(const rapidjson::Document &document) {
    Deserialize(static_cast<Graph &>(*this), document);
    blocks_state_.resize(blocks.size());
    go_.resize(blocks.size());
    std::vector<std::unordered_set<std::string>> inputs(blocks.size()), outputs(blocks.size());
    for (const auto &connection : connections) {
        if (connection.start_block_id >= blocks.size() ||
            connection.end_block_id >= blocks.size()) {
            throw ValidationError(errors::kInvalidConnection);
        }
        if (connection.start_block_id == connection.end_block_id) {
            throw ValidationError(errors::kLoopsNotSupported);
        }
        go_[connection.start_block_id].push_back(connection);
        inputs[connection.end_block_id].insert(connection.end_filename);
        outputs[connection.start_block_id].insert(connection.start_filename);
    }
    for (size_t block_id = 0; block_id < blocks.size(); ++block_id) {
        blocks_state_[block_id].cnt_inputs = inputs[block_id].size();
        std::unordered_set<std::string> filenames;
        for (const auto &bind : blocks[block_id].binds) {
            filenames.insert(bind.inside_filename);
        }
        filenames.merge(inputs[block_id]);
        filenames.merge(outputs[block_id]);
        if (!inputs[block_id].empty() || !outputs[block_id].empty()) {
            throw ValidationError(errors::kDuplicatedFilename);
        }
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
    BlockResponse block_response = {.block_id = block_id, .state = states::kRunning};
    try {
        PrepareContainer(block_id);
        SendTask(block_id, ws);
        SendToAllClients(StringifyJSON(Serialize(block_response)));
    } catch (const fs::filesystem_error &error) {
        block_response.state = states::kComplete;
        block_response.error = errors::kRuntimeErrorPrefix + error.what();
        SendToAllClients(StringifyJSON(Serialize(block_response)));
        ClearBlockState(block_id);
        partition_ptr->AddRunner(ws);
        DequeueBlock();
        UpdateBlocksProcessing();
    }
}

void GraphState::OnResult(RunnerWebSocket *ws, std::string_view message) {
    size_t block_id = ws->getUserData()->block_id;
    RunResponse run_response;
    Deserialize(run_response, ParseJSON(std::string(message)));
    BlockResponse block_response = {.block_id = block_id,
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
    SendToAllClients(StringifyJSON(Serialize(block_response)));
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
        fs::create_directory(container_path);
        fs::permissions(container_path, fs::perms::all, fs::perm_options::add);
    }
    for (const auto &bind : blocks[block_id].binds) {
        // TODO: mount
        fs::copy(bind.outside_path, container_path / bind.inside_filename,
                 fs::copy_options::create_hard_links | fs::copy_options::recursive |
                     fs::copy_options::skip_symlinks);
    }
}

bool GraphState::TransferFile(const Connection &connection) {
    std::string start_container = GetContainer(connection.start_block_id);
    std::string start_filename = connection.start_filename;
    std::string end_container = GetContainer(connection.end_block_id);
    std::string end_filename = connection.end_filename;
    fs::path start_container_path = fs::path(SANDBOX_DIR) / start_container;
    fs::path end_container_path = fs::path(SANDBOX_DIR) / end_container;
    if (!fs::exists(end_container_path)) {
        fs::create_directory(end_container_path);
        fs::permissions(end_container_path, fs::perms::all, fs::perm_options::add);
    }
    fs::path start_filepath = start_container_path / start_filename;
    fs::path end_filepath = end_container_path / end_filename;
    if (!fs::exists(start_filepath) || fs::exists(end_filepath)) {
        return false;
    }
    if (chown(start_filepath.c_str(), 0, 0)) {
        throw fs::filesystem_error("unable to chown", start_filepath,
                                   std::make_error_code(static_cast<std::errc>(errno)));
    }
    // TODO: mount
    if (connection.type == "regular") {
        fs::copy(start_filepath, end_filepath, fs::copy_options::create_hard_links);
        fs::permissions(end_filepath, fs::perms::group_write | fs::perms::others_write,
                        fs::perm_options::remove);
    } else if (connection.type == "directory") {
        fs::copy(start_filepath, end_filepath,
                 fs::copy_options::create_hard_links | fs::copy_options::recursive |
                     fs::copy_options::skip_symlinks);
        fs::permissions(end_filepath, fs::perms::group_write | fs::perms::others_write,
                        fs::perm_options::remove);
    }
    ++blocks_state_[connection.end_block_id].cnt_inputs_ready;
    return true;
}

void GraphState::SendTask(size_t block_id, RunnerWebSocket *ws) {
    RunRequest run_request = {.container = GetContainer(block_id), .task = blocks[block_id].task};
    ws->send(StringifyJSON(Serialize(run_request)));
}

bool GraphState::IsBlockReady(size_t block_id) const {
    return blocks_state_[block_id].cnt_inputs_ready == blocks_state_[block_id].cnt_inputs;
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
