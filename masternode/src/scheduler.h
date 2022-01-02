#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

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
public:
    std::string AddGraph(Graph &&graph);
    bool ExistsGraph(const std::string &graph_id);
    void RunGraph(const std::string &graph_id);
    void StopGraph(const std::string &graph_id);

    void JoinRunner(RunnerWebSocket *ws);
    void LeaveRunner(RunnerWebSocket *ws);
    void JoinUser(UserWebSocket *ws);
    void LeaveUser(UserWebSocket *ws);

private:
    std::unordered_map<std::string, Graph> graphs_;
    std::unordered_map<std::string, std::unordered_set<RunnerWebSocket *>> runners_;
    std::unordered_map<std::string, std::unordered_set<UserWebSocket *>> users_;
};
