#include "scheduler.h"
#include "uuid.h"

void SendRunRequest(const Graph::Block &block, RunnerWebSocket *ws) {
    // TODO
}

void GraphState::EnqueueBlock(size_t block_id) {
    if (cnt_blocks_processing < meta.max_runners) {
        group->EnqueueBlock(this, block_id);
        ++cnt_blocks_processing;
    } else {
        blocks_ready_.push(block_id);
    }
}

void GraphState::DequeueBlock() {
    --cnt_blocks_processing;
    if (!blocks_ready_.empty()) {
        size_t block_id = blocks_ready_.front();
        blocks_ready_.pop();
        group->EnqueueBlock(this, block_id);
    }
}

void Group::AddRunner(RunnerWebSocket *ws) {
    if (blocks_waiting_.empty()) {
        runners_waiting_.insert(ws);
    } else {
        const auto &[graph_ptr, block_id] = blocks_waiting_.front();
        blocks_waiting_.pop();
        ws->getUserData()->graph_ptr = graph_ptr;
        ws->getUserData()->block_id = block_id;
        SendRunRequest(graph_ptr->blocks[block_id], ws);
    }
}

void Group::RemoveRunner(RunnerWebSocket *ws) {
    runners_waiting_.erase(ws);
}

void Group::EnqueueBlock(GraphState *graph_ptr, size_t block_id) {
    if (runners_waiting_.empty()) {
        blocks_waiting_.emplace(graph_ptr, block_id);
    } else {
        RunnerWebSocket *ws = runners_waiting_.extract(runners_waiting_.begin()).value();
        ws->getUserData()->graph_ptr = graph_ptr;
        ws->getUserData()->block_id = block_id;
        SendRunRequest(graph_ptr->blocks[block_id], ws);
    }
}

// void Scheduler::BlockSucceeded(RunnerWebSocket *ws) {
//
// }

void Scheduler::JoinRunner(RunnerWebSocket *ws) {
    std::string runner_group = ws->getUserData()->runner_group;
    groups_[runner_group].AddRunner(ws);
}

void Scheduler::LeaveRunner(RunnerWebSocket *ws) {
    std::string runner_group = ws->getUserData()->runner_group;
    groups_[runner_group].RemoveRunner(ws);
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
    GraphState *graph_ptr = ws->getUserData()->graph_ptr;
    size_t from = ws->getUserData()->block_id;
    graph_ptr->DequeueBlock();
    for (const auto &connection : graph_ptr->go[from]) {
        int to = connection.end_block_id;
        --graph_ptr->cnt_free_inputs[to]; // TODO: check input, filesystem
        if (graph_ptr->cnt_free_inputs[to] == 0) {
            graph_ptr->EnqueueBlock(to);
        }
    }
}

std::string Scheduler::AddGraph(Graph &&graph) {
    std::string graph_id = GenerateUuid();
    GraphState graph_state = std::move(graph);
    graph_state.group = &groups_[graph_state.meta.runner_group];
    graphs_[graph_id] = std::move(graph_state);
    return graph_id;
}

bool Scheduler::GraphExists(const std::string &graph_id) {
    return graphs_.contains(graph_id);
}

void Scheduler::RunGraph(const std::string &graph_id) {
    GraphState &graph = graphs_[graph_id];
    size_t blocks_cnt = graph.blocks.size();
    graph.cnt_free_inputs.assign(blocks_cnt, 0);
    graph.cnt_ready_inputs.assign(blocks_cnt, 0);
    for (size_t block_id = 0; block_id < blocks_cnt; ++block_id) {
        for (const Graph::BlockInput &input : graph.blocks[block_id].inputs) {
            if (!input.bind_path) {
                ++graph.cnt_free_inputs[block_id];
            }
        }
        if (graph.cnt_free_inputs[block_id] == 0) {
            graph.EnqueueBlock(block_id);
        }
    }
}

bool Scheduler::GraphRunning(const std::string &graph_id) {
    return graphs_[graph_id].cnt_blocks_processing > 0;
}

void Scheduler::StopGraph(const std::string &graph_id) {
    // TODO
}
