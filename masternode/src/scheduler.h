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

struct UserPerSocketData {
    std::string graph_id;
};

using RunnerWebSocket = uWS::WebSocket<false, true, RunnerPerSocketData>;
using UserWebSocket = uWS::WebSocket<false, true, UserPerSocketData>;

void SendRunRequest(const Graph::Block &block, RunnerWebSocket *ws);

class GraphState : public Graph {
public:
    Group *group;
    std::vector<int> cnt_free_inputs, cnt_ready_inputs;
    int cnt_blocks_processing = 0;

    GraphState() = default;
    GraphState(Graph &&graph) : Graph(std::move(graph)) {
    }

    void EnqueueBlock(size_t block_id);
    void DequeueBlock();

private:
    std::queue<size_t> blocks_ready_;
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
    void JoinUser(UserWebSocket *ws);
    void LeaveUser(UserWebSocket *ws);

    void RunnerCompleted(RunnerWebSocket *ws, std::string_view message);

    std::string AddGraph(Graph &&graph);
    bool GraphExists(const std::string &graph_id);
    void RunGraph(const std::string &graph_id);
    bool GraphRunning(const std::string &graph_id);
    void StopGraph(const std::string &graph_id);

private:
    std::unordered_map<std::string, GraphState> graphs_;
    std::unordered_map<std::string, Group> groups_;
    std::unordered_map<std::string, std::unordered_set<UserWebSocket *>> users_;
};
