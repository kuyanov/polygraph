#include <chrono>
#include <filesystem>
#include <string>
#include <thread>
#include <libsbox.h>

#include "config.h"
#include "logger.h"
#include "net.h"
#include "operations.h"
#include "result.h"
#include "run.h"
#include "task.h"

namespace fs = std::filesystem;

const std::string kSchedulerHost = Config::Get().scheduler_host;
const int kSchedulerPort = Config::Get().scheduler_port;
const int kStartDelayMs = 100;

void FillTask(const Task &task, libsbox::Task &libsbox_task) {
    libsbox_task.set_argv(task.argv);
    libsbox_task.get_env() = task.env;
    if (task.stdin_.has_value()) {
        libsbox_task.get_stdin().use_file(task.stdin_.value());
    } else {
        libsbox_task.get_stdin().disable();
    }
    if (task.stdout_.has_value()) {
        libsbox_task.get_stdout().use_file(task.stdout_.value());
    } else {
        libsbox_task.get_stdout().disable();
    }
    if (task.stderr_.has_value()) {
        libsbox_task.get_stderr().use_file(task.stderr_.value());
    } else {
        libsbox_task.get_stderr().disable();
    }
    if (task.limits.time_limit_ms.has_value()) {
        libsbox_task.set_time_limit_ms(task.limits.time_limit_ms.value());
    }
    if (task.limits.wall_time_limit_ms.has_value()) {
        libsbox_task.set_wall_time_limit_ms(task.limits.wall_time_limit_ms.value());
    }
    if (task.limits.memory_limit_kb.has_value()) {
        libsbox_task.set_memory_limit_kb(task.limits.memory_limit_kb.value());
    }
    if (task.limits.fsize_limit_kb.has_value()) {
        libsbox_task.set_fsize_limit_kb(task.limits.fsize_limit_kb.value());
    }
    if (task.limits.max_files.has_value()) {
        libsbox_task.set_max_files(task.limits.max_files.value());
    }
    if (task.limits.max_threads.has_value()) {
        libsbox_task.set_max_threads(task.limits.max_threads.value());
    }
}

void FillResult(const libsbox::Task &libsbox_task, Result &result) {
    result.exited = libsbox_task.exited();
    result.signaled = libsbox_task.signaled();
    result.time_limit_exceeded = libsbox_task.is_time_limit_exceeded();
    result.wall_time_limit_exceeded = libsbox_task.is_wall_time_limit_exceeded();
    result.memory_limit_exceeded = libsbox_task.is_memory_limit_hit();
    result.oom_killed = libsbox_task.is_oom_killed();
    result.exit_code = libsbox_task.get_exit_code();
    result.term_signal = libsbox_task.get_term_signal();
    result.time_usage_ms = libsbox_task.get_time_usage_ms();
    result.time_usage_sys_ms = libsbox_task.get_time_usage_sys_ms();
    result.time_usage_user_ms = libsbox_task.get_time_usage_user_ms();
    result.wall_time_usage_ms = libsbox_task.get_wall_time_usage_ms();
    result.memory_usage_kb = libsbox_task.get_memory_usage_kb();
}

void HandleMessage(const std::string &message, WebsocketClientSession &session) {
    RunRequest run_request;
    Load(run_request, ParseJSON(message));
    auto container_path = fs::path(SANDBOX_DIR) / run_request.container;
    libsbox::Task libsbox_task;
    libsbox_task.get_binds().push_back(
        libsbox::BindRule(".", container_path.string(), libsbox::BindRule::WRITABLE));
    FillTask(run_request.task, libsbox_task);
    auto error = libsbox::run_together({&libsbox_task});
    RunResponse run_response;
    if (error) {
        run_response.error = error.get();
    } else {
        run_response.result.emplace();
        FillResult(libsbox_task, run_response.result.value());
    }
    session.Write(StringifyJSON(Dump(run_response)));
}

void Run() {
    Logger::Get().SetName("runner");
    std::this_thread::sleep_for(std::chrono::milliseconds(kStartDelayMs));
    while (true) {
        try {
            WebsocketClientSession session;
            session.Connect(kSchedulerHost, kSchedulerPort, "/runner/" + Config::Get().partition);
            Log("Connected to ", kSchedulerHost, ":", kSchedulerPort);
            session.OnRead([&](const std::string &message) { HandleMessage(message, session); });
            session.Run();
        } catch (const beast::system_error &error) {
            Log(error.what());
            std::this_thread::sleep_for(
                std::chrono::milliseconds(Config::Get().reconnect_interval_ms));
        }
    }
}
