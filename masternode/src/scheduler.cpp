#include "scheduler.h"
#include "uuid.h"

void Scheduler::RunBlock(const std::string &graph_id, size_t block_id, RunnerWebSocket *ws) {
    // TODO
}

void Scheduler::BlockSucceeded(const std::string &graph_id, size_t block_id, RunnerWebSocket *ws) {
    // TODO
}

void Scheduler::EnqueueBlocks(const std::string &graph_id) {
    auto &graph_state = graphs_[graph_id];
    auto &group_state = groups_[graph_state.graph.meta.runner_group];
    int max_runners = graph_state.graph.meta.max_runners;
    while (graph_state.cnt_blocks_processing < max_runners && !graph_state.blocks_ready.empty()) {
        size_t block_id = graph_state.blocks_ready.front();
        graph_state.blocks_ready.pop();
        group_state.blocks_waiting.emplace(graph_id, block_id);
        ++graph_state.cnt_blocks_processing;
    }
}

void Scheduler::JoinRunner(RunnerWebSocket *ws) {
    std::string runner_group = ws->getUserData()->runner_group;
    auto &group_state = groups_[runner_group];
    if (group_state.blocks_waiting.empty()) {
        group_state.runners_waiting.insert(ws);
    } else {
        const auto &[graph_id, block_id] = group_state.blocks_waiting.front();
        group_state.blocks_waiting.pop();
        RunBlock(graph_id, block_id, ws);
    }
}

void Scheduler::LeaveRunner(RunnerWebSocket *ws) {
    std::string runner_group = ws->getUserData()->runner_group;
    groups_[runner_group].runners_waiting.erase(ws);
}

void Scheduler::JoinUser(UserWebSocket *ws) {
    std::string graph_id = ws->getUserData()->graph_id;
    users_[graph_id].insert(ws);
}

void Scheduler::LeaveUser(UserWebSocket *ws) {
    std::string graph_id = ws->getUserData()->graph_id;
    users_[graph_id].erase(ws);
}

void Scheduler::RunnerCompleted(RunnerWebSocket *ws, std::string_view message) {
    // TODO
}

std::string Scheduler::AddGraph(Graph &&graph) {
    std::string graph_id = GenerateUuid();
    graphs_[graph_id] = std::move(graph);
    return graph_id;
}

bool Scheduler::GraphExists(const std::string &graph_id) {
    return graphs_.contains(graph_id);
}

void Scheduler::RunGraph(const std::string &graph_id) {
    auto &graph_state = graphs_[graph_id];
    size_t blocks_cnt = graph_state.graph.blocks.size();
    graph_state.cnt_free_inputs.assign(blocks_cnt, 0);
    graph_state.cnt_ready_inputs.assign(blocks_cnt, 0);
    for (size_t block_id = 0; block_id < blocks_cnt; ++block_id) {
        for (const auto &input : graph_state.graph.blocks[block_id].inputs) {
            if (!input.bind_path) {
                ++graph_state.cnt_free_inputs[block_id];
            }
        }
        if (graph_state.cnt_free_inputs[block_id] == 0) {
            graph_state.blocks_ready.emplace(block_id);
        }
    }
    EnqueueBlocks(graph_id);
}

bool Scheduler::GraphRunning(const std::string &graph_id) {
    return graphs_[graph_id].cnt_blocks_processing > 0;
}

void Scheduler::StopGraph(const std::string &graph_id) {
    // TODO
}
