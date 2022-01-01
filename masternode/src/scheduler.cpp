#include "scheduler.h"
#include "uuid.h"

std::string Scheduler::AddGraph(const Graph &graph) {
    std::string graph_id = GenerateUuid();
    graphs_[graph_id] = graph;
    return graph_id;
}

bool Scheduler::ExistsGraph(const std::string &graph_id) {
    return graphs_.contains(graph_id);
}

void Scheduler::RunGraph(const std::string &graph_id) {
    // TODO
}

void Scheduler::StopGraph(const std::string &graph_id) {
    // TODO
}

void Scheduler::JoinRunner(RunnerWebSocket *ws) {
    std::string runner_group = ws->getUserData()->runner_group;
    runners_[runner_group].insert(ws);
}

void Scheduler::LeaveRunner(RunnerWebSocket *ws) {
    std::string runner_group = ws->getUserData()->runner_group;
    runners_[runner_group].erase(ws);
}

void Scheduler::JoinUser(UserWebSocket *ws) {
    std::string graph_id = ws->getUserData()->graph_id;
    users_[graph_id].insert(ws);
}

void Scheduler::LeaveUser(UserWebSocket *ws) {
    std::string graph_id = ws->getUserData()->graph_id;
    users_[graph_id].erase(ws);
}
