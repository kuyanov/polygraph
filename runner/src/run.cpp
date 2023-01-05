#include <chrono>
#include <filesystem>
#include <string>
#include <thread>
#include <libsbox.h>

#include "config.h"
#include "constants.h"
#include "logger.h"
#include "net.h"
#include "run.h"
#include "serialization/all.h"
#include "structures/all.h"

namespace fs = std::filesystem;

void FillTask(const RunRequest &request, libsbox::Task &task) {
    for (const auto &bind : request.binds) {
        task.get_binds().emplace_back(bind.inside,
                                      (fs::path(paths::kVarDir) / bind.outside).string(),
                                      bind.readonly ? 0 : libsbox::BindRule::WRITABLE);
    }
    task.set_argv(request.argv);
    task.get_env() = request.env;
    task.get_stdin().disable();
    task.get_stdout().disable();
    task.get_stderr().disable();
    if (request.constraints.time_limit_ms.has_value()) {
        task.set_time_limit_ms(request.constraints.time_limit_ms.value());
    }
    if (request.constraints.wall_time_limit_ms.has_value()) {
        task.set_wall_time_limit_ms(request.constraints.wall_time_limit_ms.value());
    }
    if (request.constraints.memory_limit_kb.has_value()) {
        task.set_memory_limit_kb(request.constraints.memory_limit_kb.value());
    }
    if (request.constraints.fsize_limit_kb.has_value()) {
        task.set_fsize_limit_kb(request.constraints.fsize_limit_kb.value());
    }
    if (request.constraints.max_files.has_value()) {
        task.set_max_files(request.constraints.max_files.value());
    }
    if (request.constraints.max_threads.has_value()) {
        task.set_max_threads(request.constraints.max_threads.value());
    }
}

void FillStatus(const libsbox::Task &task, Status &status) {
    status.exited = task.exited();
    status.signaled = task.signaled();
    status.time_limit_exceeded = task.is_time_limit_exceeded();
    status.wall_time_limit_exceeded = task.is_wall_time_limit_exceeded();
    status.memory_limit_exceeded = task.is_memory_limit_hit();
    status.oom_killed = task.is_oom_killed();
    status.exit_code = task.get_exit_code();
    status.term_signal = task.get_term_signal();
    status.time_usage_ms = task.get_time_usage_ms();
    status.time_usage_sys_ms = task.get_time_usage_sys_ms();
    status.time_usage_user_ms = task.get_time_usage_user_ms();
    status.wall_time_usage_ms = task.get_wall_time_usage_ms();
    status.memory_usage_kb = task.get_memory_usage_kb();
}

RunResponse ProcessRequest(const RunRequest &request) {
    libsbox::Task task;
    FillTask(request, task);
    auto error = libsbox::run_together({&task});
    RunResponse response;
    if (error) {
        response.error = error.get();
    } else {
        response.status.emplace();
        FillStatus(task, response.status.value());
    }
    return response;
}

void Run() {
    bool connected = true;
    while (true) {
        try {
            WebsocketClientSession session;
            session.Connect(Config::Get().scheduler_host, Config::Get().scheduler_port,
                            "/runner/" + Config::Get().partition);
            connected = true;
            Log("Connected to ", Config::Get().scheduler_host, ":", Config::Get().scheduler_port);
            session.OnRead([&](const std::string &message) {
                std::thread([=, &session] {
                    RunRequest request;
                    Deserialize(request, ParseJSON(message));
                    RunResponse response = ProcessRequest(request);
                    session.Write(StringifyJSON(Serialize(response)));
                }).detach();
            });
            session.Run();
        } catch (const beast::system_error &error) {
            if (connected) {
                connected = false;
                Log("Not connected to ", Config::Get().scheduler_host, ":",
                    Config::Get().scheduler_port, ": ", error.what());
            }
            std::this_thread::sleep_for(
                std::chrono::milliseconds(Config::Get().reconnect_interval_ms));
        }
    }
}
