#include <filesystem>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>

#include "client.h"
#include "constants.h"
#include "json.h"
#include "options.h"
#include "serialization/all.h"
#include "terminal.h"
#include "uuid.h"

namespace fs = std::filesystem;

Client::Client() {
    auto document = ReadJSON(Options::Get().workflow);
    std::string body = StringifyJSON(document);
    std::string workflow_id =
        HttpSession(Options::Get().host, Options::Get().port).Post("/submit", body);
    if (!regex_match(workflow_id, std::regex(kUuidRegex))) {
        throw std::runtime_error(workflow_id);
    }
    Deserialize(workflow_, document);
    blocks_.resize(workflow_.blocks.size());
    session_.Connect(Options::Get().host, Options::Get().port, "/workflow/" + workflow_id);
    session_.OnRead([this](const std::string &message) { OnMessage(message); });
}

void Client::Run() {
    PrintWarnings();
    PrintBlocks();
    session_.Write(signals::kRun);
    session_.Run();
    PrintErrors();
}

void Client::Stop() {
    session_.Write(signals::kStop);
}

void Client::OnMessage(const std::string &message) {
    if (message == states::kComplete) {
        session_.Stop();
        return;
    } else if (message.starts_with(errors::kAPIErrorPrefix)) {
        std::cerr << message << std::endl;
        session_.Stop();
        return;
    }
    BlockResponse block;
    Deserialize(block, ParseJSON(message));
    blocks_[block.block_id] = block;
    PrintBlocks();
}

void Client::PrintWarnings() {
    for (const auto &block : workflow_.blocks) {
        for (const auto &bind : block.binds) {
            auto outside_abspath = fs::path(paths::kVarDir) / "user" / bind.outside;
            if (!fs::exists(outside_abspath)) {
                std::cerr << ColoredText("Warning: file " + bind.outside + " does not exist",
                                         YELLOW)
                          << std::endl;
                continue;
            }
            auto bind_perms = fs::status(outside_abspath).permissions();
            if ((bind_perms & fs::perms::others_read) == fs::perms::none) {
                std::cerr << ColoredText("Warning: no read permission for file " + bind.outside,
                                         YELLOW)
                          << std::endl;
            }
            if (!bind.readonly && (bind_perms & fs::perms::others_write) == fs::perms::none) {
                std::cerr << ColoredText("Warning: no write permission for file " + bind.outside,
                                         YELLOW)
                          << std::endl;
            }
        }
    }
}

void Client::PrintBlocks() {
    TerminalWindow::Get().Clear();
    for (size_t block_id = 0; block_id < blocks_.size(); ++block_id) {
        std::string line = "[block " + std::to_string(block_id) + "] ";
        const auto &block = blocks_[block_id];
        if (block.state.empty()) {
            line += "-";
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
    for (const auto &block : blocks_) {
        if (block.error.has_value()) {
            std::cerr << block.error.value() << std::endl;
        }
    }
}
