#include <filesystem>
#include <fstream>

#include "constants.h"
#include "error.h"
#include "json.h"
#include "scheduler.h"
#include "uuid.h"

GraphState::GraphState(Graph &&graph) : Graph(std::move(graph)) {
    blocks_state_.resize(blocks.size());
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
    try {
        PrepareBlockContainer(block_id);
        SendTasks(block_id, ws);
    } catch (const std::filesystem::filesystem_error &error) {
        partition_ptr->AddRunner(ws);
        SendToAllClients(errors::kRuntimeErrorPrefix + error.what());
        DequeueBlock();
        ClearBlockState(block_id);
        if (!IsRunning()) {
            SendToAllClients(signals::kGraphComplete);
        }
    }
}

void GraphState::OnComplete(RunnerWebSocket *ws, std::string_view message) {
    size_t block_id = ws->getUserData()->block_id;
    partition_ptr->AddRunner(ws);
    bool success = ProcessResults(block_id, message);
    DequeueBlock();
    if (success) {
        try {
            for (const auto &connection : connections[block_id]) {
                if (TransferFile(connection) && IsBlockReady(connection.end_block_id)) {
                    EnqueueBlock(connection.end_block_id);
                }
            }
        } catch (const std::filesystem::filesystem_error &error) {
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
    if (!std::filesystem::exists(filesystem::kSandboxPath / container_name)) {
        std::filesystem::create_directories(filesystem::kSandboxPath / container_name);
    }
    for (const auto &external : blocks[block_id].externals) {
        std::filesystem::path user_path = filesystem::kUserPath / external.user_path;
        std::filesystem::path internal_path =
            filesystem::kSandboxPath / container_name / external.name;
        if (!std::filesystem::exists(user_path)) {
            std::ofstream(user_path).close();
        }
        std::filesystem::copy(user_path, internal_path,
                              std::filesystem::copy_options::create_hard_links |
                                  std::filesystem::copy_options::recursive);
        if (external.allow_write) {
            std::filesystem::permissions(internal_path, filesystem::kWritePerms,
                                         std::filesystem::perm_options::add);
        }
        if (external.allow_exec) {
            std::filesystem::permissions(internal_path, filesystem::kExecPerms,
                                         std::filesystem::perm_options::add);
        }
    }
}

bool GraphState::TransferFile(const Connection &connection) {
    std::string start_container_name = GetContainerName(connection.start_block_id);
    std::string start_output_name =
        blocks[connection.start_block_id].outputs[connection.start_output_id].name;
    std::string end_container_name = GetContainerName(connection.end_block_id);
    std::string end_input_name =
        blocks[connection.end_block_id].inputs[connection.end_input_id].name;
    if (!std::filesystem::exists(filesystem::kSandboxPath / end_container_name)) {
        std::filesystem::create_directories(filesystem::kSandboxPath / end_container_name);
    }
    std::filesystem::path start_output_path =
        filesystem::kSandboxPath / start_container_name / start_output_name;
    std::filesystem::path end_input_path =
        filesystem::kSandboxPath / end_container_name / end_input_name;
    if (!std::filesystem::exists(start_output_path) || std::filesystem::exists(end_input_path)) {
        return false;
    }
    std::filesystem::copy(start_output_path, end_input_path,
                          std::filesystem::copy_options::create_hard_links |
                              std::filesystem::copy_options::recursive);
    if (blocks[connection.end_block_id].inputs[connection.end_input_id].allow_exec) {
        std::filesystem::permissions(end_input_path, filesystem::kExecPerms,
                                     std::filesystem::perm_options::add);
    }
    ++blocks_state_[connection.end_block_id].cnt_inputs_ready;
    return true;
}

void GraphState::SendTasks(size_t block_id, RunnerWebSocket *ws) {
    std::string container_name = GetContainerName(block_id);
    auto &alloc = blocks[block_id].tasks.GetAllocator();
    auto tasks_array = blocks[block_id].tasks.GetArray();
    for (size_t task_id = 0; task_id < tasks_array.Size(); ++task_id) {
        tasks_array[task_id]["container"].SetString(container_name.c_str(), alloc);
    }
    ws->send(StringifyJSON(blocks[block_id].tasks));
}

bool GraphState::ProcessResults(size_t block_id, std::string_view message) {
    std::string status_json(message);
    rapidjson::Document status_document;
    status_document.Parse(status_json.c_str());
    auto &alloc = status_document.GetAllocator();
    auto tasks_array = status_document.GetArray();
    bool exited_normally = true;
    for (size_t task_id = 0; task_id < tasks_array.Size(); ++task_id) {
        tasks_array[task_id].AddMember("block-id", rapidjson::Value().SetInt(block_id), alloc);
        if (!tasks_array[task_id]["exited"].GetBool() ||
            tasks_array[task_id]["exit-code"].GetInt() != 0) {
            exited_normally = false;
        }
    }
    SendToAllClients(StringifyJSON(status_document));
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

std::string Scheduler::AddGraph(Graph &&graph) {
    std::string graph_id = GenerateUuid();
    GraphState graph_state = std::move(graph);
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
