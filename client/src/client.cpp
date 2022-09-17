#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "client.h"
#include "config.h"
#include "constants.h"
#include "operations.h"
#include "uuid.h"

const std::string kSchedulerHost = Config::Get().scheduler_host;
const int kSchedulerPort = Config::Get().scheduler_port;
const int kTerminalWidth = 50;

Client::Client(const std::string &graph_path) {
    auto document = ReadJSON(graph_path);
    std::string body = StringifyJSON(document);
    std::string uuid = HttpSession(kSchedulerHost, kSchedulerPort).Post("/submit", body);
    if (!IsUuid(uuid)) {
        throw std::runtime_error(uuid);
    }
    session_.Connect(kSchedulerHost, kSchedulerPort, "/graph/" + uuid);
    session_.OnRead([this](const std::string &message) { HandleMessage(message); });
    Load(graph_, document);
    blocks_.resize(graph_.blocks.size());
}

void Client::Run() {
    PrintBlocks();
    session_.Write(signals::kRun);
    session_.Run();
}

void Client::Stop() {
    session_.Write(signals::kStop);
}

void Client::HandleMessage(const std::string &message) {
    if (message == states::kComplete) {
        PrintErrors();
        session_.Stop();
        return;
    } else if (message.starts_with(errors::kAPIErrorPrefix)) {
        std::cerr << message << std::endl;
        PrintErrors();
        session_.Stop();
        return;
    }
    BlockResponse block;
    Load(block, ParseJSON(message));
    blocks_[block.block_id] = block;
    PrintBlocks();
}

void Client::PrintBlocks() {
    if (blocks_printed_) {
        for (const auto &block : blocks_) {
            std::cout << "\x1b[1A";
        }
        for (const auto &block : blocks_) {
            for (int i = 0; i < kTerminalWidth; ++i) {
                std::cout << ' ';
            }
            std::cout << std::endl;
        }
        for (const auto &block : blocks_) {
            std::cout << "\x1b[1A";
        }
    } else {
        blocks_printed_ = true;
    }
    for (size_t block_id = 0; block_id < blocks_.size(); ++block_id) {
        std::cout << "[block " << block_id << "] ";
        const auto &block = blocks_[block_id];
        if (block.state.empty()) {
            std::cout << "Pending" << std::endl;
        } else if (block.state == states::kRunning) {
            std::cout << "Running" << std::endl;
        } else if (block.error.has_value()) {
            std::cout << "\033[31m";
            std::cout << "Error  ";
            std::cout << "\033[0m" << std::endl;
        } else if (block.result->exited) {
            std::cout << (block.result->exit_code == 0 ? "\033[32m" : "\033[33m");
            std::cout << "Exited with code " << block.result->exit_code;
            std::cout << "\033[0m" << std::endl;
        } else if (block.result->time_limit_exceeded) {
            std::cout << "\033[33m";
            std::cout << "Time limit exceeded";
            std::cout << "\033[0m" << std::endl;
        } else if (block.result->wall_time_limit_exceeded) {
            std::cout << "\033[33m";
            std::cout << "Wall time limit exceeded";
            std::cout << "\033[0m" << std::endl;
        } else if (block.result->memory_limit_exceeded) {
            std::cout << "\033[33m";
            std::cout << "Memory limit exceeded";
            std::cout << "\033[0m" << std::endl;
        } else if (block.result->oom_killed) {
            std::cout << "\033[33m";
            std::cout << "OOM killed";
            std::cout << "\033[0m" << std::endl;
        } else if (block.result->signaled) {
            std::cout << "\033[33m";
            std::cout << "Terminated with signal " << block.result->term_signal;
            std::cout << "\033[0m" << std::endl;
        } else {
            std::cout << std::endl;
        }
    }
}

void Client::PrintErrors() {
    for (size_t block_id = 0; block_id < blocks_.size(); ++block_id) {
        const auto &error = blocks_[block_id].error;
        if (error.has_value()) {
            std::cerr << "[block " << block_id << "] " << error.value() << std::endl;
        }
    }
}
