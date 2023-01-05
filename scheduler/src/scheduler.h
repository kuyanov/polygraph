#pragma once

#include <optional>
#include <queue>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <rapidjson/document.h>
#include <App.h>

#include "structures/workflow.h"

class WorkflowState;
class Partition;

struct RunnerPerSocketData {
    std::string partition;
    WorkflowState *workflow_ptr;
    size_t block_id;
};

struct ClientPerSocketData {
    WorkflowState *workflow_ptr;
};

using RunnerWebSocket = uWS::WebSocket<false, true, RunnerPerSocketData>;
using ClientWebSocket = uWS::WebSocket<false, true, ClientPerSocketData>;

class WorkflowState : public Workflow {
public:
    std::string workflow_id;
    Partition *partition_ptr;

    WorkflowState() = default;

    void Init(const rapidjson::Document &document);

    void Run();
    void Stop();

    void RunBlock(size_t block_id, RunnerWebSocket *ws);
    void OnStatus(RunnerWebSocket *ws, std::string_view message);

    void EnqueueBlock(size_t block_id);
    void DequeueBlock();
    void UpdateBlocksProcessing();

    bool ProcessConnection(const Connection &connection);
    bool IsBlockReady(size_t block_id) const;

    void PrepareRun(size_t block_id);
    void FinalizeRun(size_t block_id);

    void SendRunRequest(size_t block_id, RunnerWebSocket *ws);

    void AddClient(ClientWebSocket *ws);
    void RemoveClient(ClientWebSocket *ws);
    void SendToAllClients(std::string_view message);

private:
    struct BlockState {
        size_t cnt_inputs_ready = 0;
        size_t cnt_runs = 0;
        std::vector<std::optional<std::string>> input_sources;
    };

    bool is_running_ = false;
    size_t cnt_blocks_processing_ = 0;
    std::queue<size_t> blocks_ready_;
    std::vector<BlockState> blocks_state_;
    std::vector<std::vector<Connection>> go_;
    std::unordered_set<ClientWebSocket *> clients_;

    std::string GetContainerId(size_t block_id) const;
};

class Partition {
public:
    void AddRunner(RunnerWebSocket *ws);
    void RemoveRunner(RunnerWebSocket *ws);
    void EnqueueBlock(WorkflowState *workflow_ptr, size_t block_id);

private:
    std::unordered_set<RunnerWebSocket *> runners_waiting_;
    std::queue<std::pair<WorkflowState *, size_t>> blocks_waiting_;
};

class Scheduler {
public:
    void JoinRunner(RunnerWebSocket *ws);
    void LeaveRunner(RunnerWebSocket *ws);
    void JoinClient(ClientWebSocket *ws);
    void LeaveClient(ClientWebSocket *ws);

    std::string AddWorkflow(const rapidjson::Document &document);
    WorkflowState *FindWorkflow(const std::string &workflow_id);

private:
    std::unordered_map<std::string, WorkflowState> workflows_;
    std::unordered_map<std::string, Partition> groups_;
};
