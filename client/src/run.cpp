#include <iostream>
#include <string>
#include <vector>

#include "config.h"
#include "constants.h"
#include "graph.h"
#include "net.h"
#include "operations.h"
#include "run.h"
#include "result.h"
#include "uuid.h"

const std::string kSchedulerHost = Config::Get().scheduler_host;
const int kSchedulerPort = Config::Get().scheduler_port;
const int kTerminalWidth = 50;

void PrintBlocks(const std::vector<BlockResponse> &block_states, bool first) {
    if (!first) {
        for (const auto &block_state : block_states) {
            std::cout << "\x1b[1A";
        }
        for (const auto &block_state : block_states) {
            for (int i = 0; i < kTerminalWidth; ++i) {
                std::cout << ' ';
            }
            std::cout << std::endl;
        }
        for (const auto &block_state : block_states) {
            std::cout << "\x1b[1A";
        }
    }
    for (size_t block_id = 0; block_id < block_states.size(); ++block_id) {
        std::cout << "[block " << block_id << "] ";
        const auto &block_state = block_states[block_id];
        if (block_state.state.empty()) {
            std::cout << "Pending" << std::endl;
        } else if (block_state.state == states::kRunning) {
            std::cout << "Running" << std::endl;
        } else if (block_state.error.has_value()) {
            std::cout << "\033[31m";
            std::cout << "Error  ";
            std::cout << "\033[0m" << std::endl;
        } else if (block_state.result->exited) {
            std::cout << (block_state.result->exit_code == 0 ? "\033[32m" : "\033[33m");
            std::cout << "Exited with code " << block_state.result->exit_code;
            std::cout << "\033[0m" << std::endl;
        } else if (block_state.result->time_limit_exceeded) {
            std::cout << "\033[33m";
            std::cout << "Time limit exceeded";
            std::cout << "\033[0m" << std::endl;
        } else if (block_state.result->wall_time_limit_exceeded) {
            std::cout << "\033[33m";
            std::cout << "Wall time limit exceeded";
            std::cout << "\033[0m" << std::endl;
        } else if (block_state.result->memory_limit_exceeded) {
            std::cout << "\033[33m";
            std::cout << "Memory limit exceeded";
            std::cout << "\033[0m" << std::endl;
        } else if (block_state.result->oom_killed) {
            std::cout << "\033[33m";
            std::cout << "OOM killed";
            std::cout << "\033[0m" << std::endl;
        } else if (block_state.result->signaled) {
            std::cout << "\033[33m";
            std::cout << "Terminated with signal " << block_state.result->term_signal;
            std::cout << "\033[0m" << std::endl;
        } else {
            std::cout << std::endl;
        }
    }
}

void PrintErrors(const std::vector<BlockResponse> &block_states) {
    for (size_t block_id = 0; block_id < block_states.size(); ++block_id) {
        const auto &error = block_states[block_id].error;
        if (error.has_value()) {
            std::cout << "[block " << block_id << "] " << error.value() << std::endl;
        }
    }
}

void Run(const std::string &graph_path) {
    auto document = ReadJSON(graph_path);
    std::string body = StringifyJSON(document);
    std::string uuid = HttpSession(kSchedulerHost, kSchedulerPort).Post("/submit", body);
    if (!IsUuid(uuid)) {
        std::cerr << "Failed to run, got error response '" << uuid << "'" << std::endl;
        exit(1);
    }
    Graph graph;
    Load(graph, document);
    std::vector<BlockResponse> block_states(graph.blocks.size());
    PrintBlocks(block_states, true);
    WebsocketClientSession session;
    session.Connect(kSchedulerHost, kSchedulerPort, "/graph/" + uuid);
    session.Write(signals::kRun);
    session.OnRead([&](const std::string &message) {
        if (message == states::kComplete) {
            PrintErrors(block_states);
            session.Stop();
            return;
        }
        BlockResponse block_response;
        Load(block_response, ParseJSON(message));
        block_states[block_response.block_id] = block_response;
        PrintBlocks(block_states, false);
    });
    session.Run();
}
