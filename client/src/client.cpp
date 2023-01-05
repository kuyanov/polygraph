#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>

#include "client.h"
#include "constants.h"
#include "json.h"
#include "serialization/all.h"
#include "terminal.h"
#include "uuid.h"

Client::Client(const rapidjson::Document &document, const std::string &scheduler_host,
               int scheduler_port) {
    std::string body = StringifyJSON(document);
    std::string workflow_id = HttpSession(scheduler_host, scheduler_port).Post("/submit", body);
    if (!regex_match(workflow_id, std::regex(kUuidRegex))) {
        throw std::runtime_error(workflow_id);
    }
    Deserialize(workflow_, document);
    blocks_.resize(workflow_.blocks.size());
    session_.Connect(scheduler_host, scheduler_port, "/workflow/" + workflow_id);
    session_.OnRead([this](const std::string &message) { OnMessage(message); });
}

void Client::Run() {
    PrintBlocks();
    session_.Write(signals::kRun);
    session_.Run();
}

void Client::Stop() {
    session_.Write(signals::kStop);
}

void Client::OnMessage(const std::string &message) {
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
        } else if (block.status->exited) {
            int exit_code = block.status->exit_code;
            line += ColoredText("Exited with code " + std::to_string(exit_code),
                                exit_code == 0 ? GREEN : YELLOW);
        } else if (block.status->time_limit_exceeded) {
            line += ColoredText("Time limit exceeded", YELLOW);
        } else if (block.status->wall_time_limit_exceeded) {
            line += ColoredText("Wall time limit exceeded", YELLOW);
        } else if (block.status->memory_limit_exceeded) {
            line += ColoredText("Memory limit exceeded", YELLOW);
        } else if (block.status->oom_killed) {
            line += ColoredText("OOM killed", YELLOW);
        } else if (block.status->signaled) {
            int term_signal = block.status->term_signal;
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
