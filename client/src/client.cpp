#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include "client.h"
#include "config.h"
#include "constants.h"
#include "serialize.h"
#include "terminal.h"
#include "uuid.h"

const std::string kSchedulerHost = Config::Get().scheduler_host;
const int kSchedulerPort = Config::Get().scheduler_port;

Client::Client(const std::string &graph_path) {
    auto document = ReadJSON(graph_path);
    std::string body = StringifyJSON(document);
    std::string graph_id = HttpSession(kSchedulerHost, kSchedulerPort).Post("/submit", body);
    if (!regex_match(graph_id, std::regex(kUuidRegex))) {
        throw std::runtime_error(graph_id);
    }
    Deserialize(graph_, document);
    blocks_.resize(graph_.blocks.size());
    session_.Connect(kSchedulerHost, kSchedulerPort, "/graph/" + graph_id);
    session_.OnRead([this](const std::string &message) { HandleMessage(message); });
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
    Deserialize(block, ParseJSON(message));
    blocks_[block.block_id] = block;
    PrintBlocks();
}

void Client::PrintBlocks() {
    TerminalWindow::Get().Clear();
    for (size_t block_id = 0; block_id < blocks_.size(); ++block_id) {
        std::string line = "[block " + std::to_string(block_id) + "] ";
        const auto &block = blocks_[block_id];
        if (block.state.empty()) {
            line += ColoredText("Pending", GREY);
        } else if (block.state == states::kRunning) {
            line += "Running";
        } else if (block.error.has_value()) {
            line += ColoredText("Error", RED);
        } else if (block.result->exited) {
            int exit_code = block.result->exit_code;
            line += ColoredText("Exited with code " + std::to_string(exit_code),
                                exit_code == 0 ? GREEN : YELLOW);
        } else if (block.result->time_limit_exceeded) {
            line += ColoredText("Time limit exceeded", YELLOW);
        } else if (block.result->wall_time_limit_exceeded) {
            line += ColoredText("Wall time limit exceeded", YELLOW);
        } else if (block.result->memory_limit_exceeded) {
            line += ColoredText("Memory limit exceeded", YELLOW);
        } else if (block.result->oom_killed) {
            line += ColoredText("OOM killed", YELLOW);
        } else if (block.result->signaled) {
            int term_signal = block.result->term_signal;
            line += ColoredText("Terminated with signal " + std::to_string(term_signal), YELLOW);
        }
        TerminalWindow::Get().PrintLine(line);
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
