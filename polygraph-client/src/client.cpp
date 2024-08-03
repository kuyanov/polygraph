#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <regex>
#include <string>

#include "client.h"
#include "config.h"
#include "definitions.h"
#include "json.h"
#include "serialization/all.h"
#include "terminal.h"

namespace fs = std::filesystem;

Client::Client(const std::string &workflow_file) {
    auto document = ReadJSON(workflow_file);
    std::string body = StringifyJSON(document);
    std::string workflow_id =
        HttpSession(ClientConfig::Get().host, ClientConfig::Get().port).Post("/submit", body);
    if (!regex_match(workflow_id, std::regex(UUID_REGEX))) {
        std::cerr << "Process terminated with the following exception:" << std::endl;
        std::cerr << std::endl;
        std::cerr << workflow_id << std::endl;
        exit(EXIT_FAILURE);
    }
    Deserialize(workflow_, document);
    blocks_.resize(workflow_.blocks.size());
    cnt_runs_.resize(workflow_.blocks.size());
    session_.Connect(ClientConfig::Get().host, ClientConfig::Get().port,
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

std::string GetExecutionStatus(const BlockResponse &block, size_t cnt_runs) {
    if (block.state.empty()) {
        return "-";
    } else if (block.state == RUNNING_STATE) {
        std::string status = "Running";
        if (cnt_runs != 1) {
            status += " (" + std::to_string(cnt_runs) + ")";
        }
        return status;
    } else if (block.error.has_value()) {
        return ColoredText("Error", RED);
    } else if (block.status->exited) {
        int exit_code = block.status->exit_code;
        return ColoredText("Exited with code " + std::to_string(exit_code),
                           exit_code == 0 ? GREEN : YELLOW);
    } else if (block.status->time_limit_exceeded) {
        return ColoredText("Time limit exceeded", YELLOW);
    } else if (block.status->wall_time_limit_exceeded) {
        return ColoredText("Wall time limit exceeded", YELLOW);
    } else if (block.status->memory_limit_exceeded) {
        return ColoredText("Memory limit exceeded", YELLOW);
    } else if (block.status->oom_killed) {
        return ColoredText("OOM killed", YELLOW);
    } else if (block.status->signaled) {
        int term_signal = block.status->term_signal;
        return ColoredText("Terminated with signal " + std::to_string(term_signal), YELLOW);
    }
    return "";
}

std::string GetTimeUsage(const BlockResponse &block) {
    std::stringstream time_usage;
    time_usage << std::setprecision(3) << std::fixed;
    if (block.status.has_value() && block.status->time_usage_ms != -1) {
        time_usage << block.status->time_usage_ms / 1000.0;
    }
    return time_usage.str();
}

std::string GetMemoryUsage(const BlockResponse &block) {
    std::stringstream memory_usage;
    memory_usage << std::setprecision(3) << std::fixed;
    if (block.status.has_value() && block.status->memory_usage_kb != -1) {
        memory_usage << block.status->memory_usage_kb / 1024.0;
    }
    return memory_usage.str();
}

void Client::PrintBlocks() {
    TerminalWindow::Get().Clear();
    size_t name_width = 0;
    for (const auto &block : workflow_.blocks) {
        name_width = std::max(name_width, block.name.length());
    }
    size_t status_width = 26;
    size_t time_width = 10;
    size_t memory_width = 12;
    std::string header;
    header += AlignCenter("", name_width + 2);
    header += " ";
    header += AlignLeft("Status", status_width);
    header += "  ";
    header += AlignLeft("Time (sec)", time_width);
    header += "  ";
    header += AlignLeft("Memory (MiB)", memory_width);
    TerminalWindow::Get().PrintLine(header);
    for (size_t block_id = 0; block_id < blocks_.size(); ++block_id) {
        std::string line;
        line += "[" + AlignCenter(workflow_.blocks[block_id].name, name_width) + "]";
        line += " ";
        line += AlignLeft(GetExecutionStatus(blocks_[block_id], cnt_runs_[block_id]), status_width);
        line += "  ";
        line += AlignLeft(GetTimeUsage(blocks_[block_id]), time_width);
        line += "  ";
        line += AlignLeft(GetMemoryUsage(blocks_[block_id]), memory_width);
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
