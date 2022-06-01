#pragma once

#include <queue>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <App.h>

#include "graph.h"

class GraphState;
class RunnerGroup;

struct RunnerPerSocketData {
    std::string runner_group;
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
    RunnerGroup *runner_group_ptr;

    GraphState() = default;
    GraphState(Graph &&graph);

    void Run();
    void Stop();
    bool IsRunning() const;

    void RunBlock(size_t block_id, RunnerWebSocket *ws);
    void CompleteBlock(RunnerWebSocket *ws, std::string_view message);

    void EnqueueBlock(size_t block_id);
    void DequeueBlock();

    void PrepareBlockContainer(size_t block_id);
    bool TransferFile(const Connection &connection);
    bool BlockReadyToRun(size_t block_id) const;

    void AddClient(ClientWebSocket *ws);
    void RemoveClient(ClientWebSocket *ws);
    void SendToAllClients(std::string_view message);

private:
    struct BlockState {
        size_t cnt_inputs_ready = 0;
        size_t cnt_runs = 0;
    };

    int cnt_blocks_processing_ = 0;
    std::queue<size_t> blocks_ready_;
    std::vector<BlockState> blocks_state_;
    std::unordered_set<ClientWebSocket *> clients_;

    std::string GetContainerName(size_t block_id) const;
};

class RunnerGroup {
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

    std::string AddGraph(Graph &&graph);
    GraphState *FindGraph(const std::string &graph_id);

private:
    std::unordered_map<std::string, GraphState> graphs_;
    std::unordered_map<std::string, RunnerGroup> groups_;
};
