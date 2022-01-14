#include "scheduler.h"
#include "uuid.h"

void SendRunRequest(const Graph::Block &block, RunnerWebSocket *ws) {
    ws->send("hello");  // TODO
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
    if (!blocks_ready_.empty()) {
        size_t block_id = blocks_ready_.front();
        blocks_ready_.pop();
        group->EnqueueBlock(this, block_id);
    } else {
        --cnt_blocks_processing;
    }
}

void GraphState::ResetInputs(size_t block_id) {
    const auto &inputs = blocks[block_id].inputs;
    is_input_ready_[block_id].assign(inputs.size(), true);
    cnt_unready_inputs_[block_id] = 0;
    for (size_t input_id = 0; input_id < inputs.size(); ++input_id) {
        if (!inputs[input_id].bind_path) {
            is_input_ready_[block_id][input_id] = false;
            ++cnt_unready_inputs_[block_id];
        }
    }
}

bool GraphState::SetInputReady(size_t block_id, size_t input_id) {
    if (!is_input_ready_[block_id][input_id]) {
        is_input_ready_[block_id][input_id] = true;
        --cnt_unready_inputs_[block_id];
        return true;
    }
    return false;
}

bool GraphState::AllInputsReady(size_t block_id) const {
    return cnt_unready_inputs_[block_id] == 0;
}

void GraphState::AddUser(UserWebSocket *ws) {
    users_.insert(ws);
}

void GraphState::RemoveUser(UserWebSocket *ws) {
    users_.erase(ws);
}

void GraphState::SendToAllUsers(std::string_view message) {
    for (UserWebSocket *ws : users_) {
        ws->send(message);
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
    graphs_[graph_id].AddUser(ws);
}

void Scheduler::LeaveUser(UserWebSocket *ws) {
    std::string graph_id = ws->getUserData()->graph_id;
    graphs_[graph_id].RemoveUser(ws);
}

std::string Scheduler::AddGraph(Graph &&graph) {
    std::string graph_id = GenerateUuid();
    GraphState graph_state = std::move(graph);
    graph_state.group = &groups_[graph_state.meta.runner_group];
    graphs_[graph_id] = std::move(graph_state);
    return graph_id;
}

bool Scheduler::GraphExists(const std::string &graph_id) const {
    return graphs_.contains(graph_id);
}

void Scheduler::RunGraph(const std::string &graph_id) {
    auto &graph = graphs_[graph_id];
    for (size_t block_id = 0; block_id < graph.blocks.size(); ++block_id) {
        graph.ResetInputs(block_id);
        if (graph.AllInputsReady(block_id)) {
            graph.EnqueueBlock(block_id);
        }
    }
    if (graph.cnt_blocks_processing == 0) {
        graph.SendToAllUsers("complete");
    }
}

void Scheduler::StopGraph(const std::string &graph_id) {
    // TODO
}

bool Scheduler::GraphRunning(const std::string &graph_id) const {
    auto it = graphs_.find(graph_id);
    return it != graphs_.end() && it->second.cnt_blocks_processing > 0;
}

void Scheduler::RunnerCompleted(RunnerWebSocket *ws, std::string_view message) {
    GraphState *graph_ptr = ws->getUserData()->graph_ptr;
    size_t start_block_id = ws->getUserData()->block_id;
    graph_ptr->group->AddRunner(ws);
    graph_ptr->SendToAllUsers(message);  // TODO
    graph_ptr->DequeueBlock();
    graph_ptr->ResetInputs(start_block_id);
    for (const auto &[_, start_output_id, end_block_id, end_input_id] :
         graph_ptr->go[start_block_id]) {
        if (graph_ptr->SetInputReady(end_block_id, end_input_id) &&
            graph_ptr->AllInputsReady(end_block_id)) {
            graph_ptr->EnqueueBlock(end_block_id);
        }
    }
    if (graph_ptr->cnt_blocks_processing == 0) {
        graph_ptr->SendToAllUsers("complete");
    }
}
