#pragma once

#include <queue>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <App.h>

#include "graph.h"

struct RunnerPerSocketData {
    std::string runner_group;
};

struct UserPerSocketData {
    std::string graph_id;
};

using RunnerWebSocket = uWS::WebSocket<false, true, RunnerPerSocketData>;
using UserWebSocket = uWS::WebSocket<false, true, UserPerSocketData>;

class Scheduler {
private:
    struct GraphState {
        Graph graph;
        std::vector<int> cnt_free_inputs, cnt_ready_inputs;
        std::queue<size_t> blocks_ready;
        int cnt_blocks_processing = 0;

        GraphState() = default;
        GraphState(Graph &&graph) : graph(std::move(graph)) {
        }
    };

    struct GroupState {
        std::unordered_set<RunnerWebSocket *> runners_waiting;
        std::queue<std::pair<std::string, size_t>> blocks_waiting;
    };

    void RunBlock(const std::string &graph_id, size_t block_id, RunnerWebSocket *ws);
    void BlockSucceeded(const std::string &graph_id, size_t block_id, RunnerWebSocket *ws);

    void EnqueueBlocks(const std::string &graph_id);

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
    std::unordered_map<std::string, GroupState> groups_;
    std::unordered_map<std::string, std::unordered_set<UserWebSocket *>> users_;
};
