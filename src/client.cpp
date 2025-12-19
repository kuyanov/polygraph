#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>

#include "block_response.h"
#include "client.h"
#include "config.h"
#include "definitions.h"
#include "json.h"
#include "submit_response.h"
#include "terminal.h"

namespace fs = std::filesystem;

Client::Client(const std::string &workflow_file) {
    auto document = ReadJSON(workflow_file);
    std::string body = StringifyJSON(document);
    std::string submit_response_text =
        HttpSession(Config::Get().host, Config::Get().port).Post("/submit", body);
    SubmitResponse submit_response;
    Deserialize(submit_response, ParseJSON(submit_response_text));
    if (submit_response.status != SUBMIT_ACCEPTED) {
        std::cerr << "While sending the workflow, the following exception occurred:" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Exception type: " << submit_response.status << std::endl;
        std::cerr << "Message: " << submit_response.data << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string workflow_id = submit_response.data;
    Deserialize(workflow_, document);
    blocks_.resize(workflow_.blocks.size());
    cnt_runs_.resize(workflow_.blocks.size());
    session_.Connect(Config::Get().host, Config::Get().port, "/workflow/" + workflow_id);
    session_.OnRead([this](const std::string &message) { OnMessage(message); });
}

void ClientInterruptHandler(int signum) {
    Client::Get("").Stop();
}

void Client::Run() {
    signal(SIGINT, ClientInterruptHandler);
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
    if (message.starts_with(WORKFLOW_SIGNAL)) {
        std::string workflow_state = message.substr(strlen(WORKFLOW_SIGNAL) + 1);
        if (workflow_state == FINISHED_STATE) {
            session_.Stop();
            return;
        }
    } else if (message.starts_with(BLOCK_SIGNAL)) {
        std::string block_response_text = message.substr(strlen(BLOCK_SIGNAL) + 1);
        BlockResponse block;
        Deserialize(block, ParseJSON(block_response_text));
        blocks_[block.block_id] = block;
        if (block.state == RUNNING_STATE) {
            ++cnt_runs_[block.block_id];
        }
        PrintBlocks();
    } else if (message.starts_with(ERROR_SIGNAL)) {
        std::string error = message.substr(strlen(ERROR_SIGNAL) + 1);
        std::cerr << "Error: " << error << std::endl;
        session_.Stop();
        return;
    }
}

void Client::PrintWarnings() {
    for (const auto &block : workflow_.blocks) {
        for (const auto &bind : block.binds) {
            if (!fs::exists(bind.outside)) {
                std::cerr << ColoredText("Warning: file " + bind.outside + " does not exist",
                                         YELLOW)
                          << std::endl;
                continue;
            }
            auto bind_perms = fs::status(bind.outside).permissions();
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
