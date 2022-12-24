#pragma once

#include <queue>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <rapidjson/document.h>
#include <uWebSockets/App.h>

#include "graph.h"

class GraphState;
class Partition;

struct RunnerPerSocketData {
    std::string partition;
    GraphState *graph_ptr;
    size_t block_id;
};

struct ClientPerSocketData {
    GraphState *graph_ptr;
};

using RunnerWebSocket = uWS::WebSocket<false, true, RunnerPerSocketData>;
using ClientWebSocket = uWS::WebSocket<false, true, ClientPerSocketData>;

class GraphState : public Graph {
public:
    std::string graph_id;
    Partition *partition_ptr;

    GraphState() = default;

    void Init(const rapidjson::Document &document);

    void Run();
    void Stop();

    void RunBlock(size_t block_id, RunnerWebSocket *ws);
    void OnResult(RunnerWebSocket *ws, std::string_view message);

    void EnqueueBlock(size_t block_id);
    void DequeueBlock();
    void UpdateBlocksProcessing();

    void PrepareContainer(size_t block_id);
    bool TransferFile(const Connection &connection);

    void SendTask(size_t block_id, RunnerWebSocket *ws);

    bool IsBlockReady(size_t block_id) const;
    void ClearBlockState(size_t block_id);

    void AddClient(ClientWebSocket *ws);
    void RemoveClient(ClientWebSocket *ws);
    void SendToAllClients(std::string_view message);

private:
    struct BlockState {
        size_t cnt_inputs = 0;
        size_t cnt_inputs_ready = 0;
        size_t cnt_runs = 0;
    };

    bool is_running_ = false;
    size_t cnt_blocks_processing_ = 0;
    std::queue<size_t> blocks_ready_;
    std::vector<BlockState> blocks_state_;
    std::vector<std::vector<Connection>> go_;
    std::unordered_set<ClientWebSocket *> clients_;

    std::string GetContainer(size_t block_id) const;
};

class Partition {
public:
    void AddRunner(RunnerWebSocket *ws);
    void RemoveRunner(RunnerWebSocket *ws);
    void EnqueueBlock(GraphState *graph_ptr, size_t block_id);

private:
    std::unordered_set<RunnerWebSocket *> runners_waiting_;
    std::queue<std::pair<GraphState *, size_t>> blocks_waiting_;
};

class Scheduler {
public:
    void JoinRunner(RunnerWebSocket *ws);
    void LeaveRunner(RunnerWebSocket *ws);
    void JoinClient(ClientWebSocket *ws);
    void LeaveClient(ClientWebSocket *ws);

    std::string AddGraph(const rapidjson::Document &document);
    GraphState *FindGraph(const std::string &graph_id);

private:
    std::unordered_map<std::string, GraphState> graphs_;
    std::unordered_map<std::string, Partition> groups_;
};
