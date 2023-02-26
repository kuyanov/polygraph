#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>

#include "client.h"
#include "config.h"
#include "definitions.h"
#include "serialization/all.h"
#include "terminal.h"

namespace fs = std::filesystem;

Client::Client(const std::string &workflow_path) {
    auto document = ReadJSON(workflow_path);
    std::string body = StringifyJSON(document);
    std::string workflow_id =
        HttpSession(Config::Get().scheduler_host, Config::Get().scheduler_port)
            .Post("/submit", body);
    if (!regex_match(workflow_id, std::regex(UUID_REGEX))) {
        std::cerr << "Process terminated with the following exception:" << std::endl;
        std::cerr << std::endl;
        std::cerr << workflow_id << std::endl;
        exit(EXIT_FAILURE);
    }
    Deserialize(workflow_, document);
    blocks_.resize(workflow_.blocks.size());
    cnt_runs_.resize(workflow_.blocks.size());
    session_.Connect(Config::Get().scheduler_host, Config::Get().scheduler_port,
                     "/workflow/" + workflow_id);
    session_.OnRead([this](const std::string &message) { OnMessage(message); });
}

void Client::Run() {
    PrintWarnings();
    PrintBlocks();
    session_.Write(RUN_SIGNAL);
    session_.Run();
    PrintErrors();
}

void Client::Stop() {
    session_.Write(STOP_SIGNAL);
}

void Client::OnMessage(const std::string &message) {
    if (message == COMPLETE_STATE) {
        session_.Stop();
        return;
    } else if (message.starts_with(API_ERROR_PREFIX)) {
        std::cerr << message << std::endl;
        session_.Stop();
        return;
    }
    BlockResponse block;
    Deserialize(block, ParseJSON(message));
    blocks_[block.block_id] = block;
    if (block.state == RUNNING_STATE) {
        ++cnt_runs_[block.block_id];
    }
    PrintBlocks();
}

void Client::PrintWarnings() {
    for (const auto &block : workflow_.blocks) {
        for (const auto &bind : block.binds) {
            auto outside_abspath = fs::path(GetVarDir()) / "user" / bind.outside;
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
    size_t max_name_width = 0;
    for (const auto &block : workflow_.blocks) {
        max_name_width = std::max(max_name_width, block.name.length());
    }
    for (size_t block_id = 0; block_id < blocks_.size(); ++block_id) {
        std::string block_name = workflow_.blocks[block_id].name;
        size_t left_padding = (max_name_width - block_name.length()) / 2;
        size_t right_padding = (max_name_width - block_name.length() + 1) / 2;
        std::string line = "[" + std::string(left_padding, ' ') + block_name +
                           std::string(right_padding, ' ') + "] ";
        const auto &block = blocks_[block_id];
        if (block.state.empty()) {
            line += "-";
        } else if (block.state == RUNNING_STATE) {
            line += "Running";
            if (cnt_runs_[block_id] != 1) {
                line += " (" + std::to_string(cnt_runs_[block_id]) + ")";
            }
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
