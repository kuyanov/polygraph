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
class Group;

struct RunnerPerSocketData {
    std::string runner_group;
    GraphState *graph_ptr;
    size_t block_id;
};

struct ClientPerSocketData {
    std::string graph_id;
};

using RunnerWebSocket = uWS::WebSocket<false, true, RunnerPerSocketData>;
using ClientWebSocket = uWS::WebSocket<false, true, ClientPerSocketData>;

void SendRunRequest(const Graph::Block &block, RunnerWebSocket *ws);

class GraphState : public Graph {
public:
    Group *group;
    int cnt_blocks_processing = 0;

    GraphState() = default;
    GraphState(Graph &&graph);

    void EnqueueBlock(size_t block_id);
    void DequeueBlock();

    void ResetInputs(size_t block_id);
    bool SetInputReady(size_t block_id, size_t input_id);
    bool AllInputsReady(size_t block_id) const;

    void AddClient(ClientWebSocket *ws);
    void RemoveClient(ClientWebSocket *ws);
    void SendToAllClients(std::string_view message);

private:
    std::queue<size_t> blocks_ready_;
    std::vector<std::vector<bool>> is_input_ready_;
    std::vector<size_t> cnt_missing_inputs_;
    std::unordered_set<ClientWebSocket *> clients_;
};

class Group {
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
    bool GraphExists(const std::string &graph_id) const;
    void RunGraph(const std::string &graph_id);
    void StopGraph(const std::string &graph_id);
    bool GraphRunning(const std::string &graph_id) const;

    void RunnerCompleted(RunnerWebSocket *ws, std::string_view message);

private:
    std::unordered_map<std::string, GraphState> graphs_;
    std::unordered_map<std::string, Group> groups_;
};
