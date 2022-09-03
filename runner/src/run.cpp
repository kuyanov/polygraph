#include <chrono>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>
#include <libsbox.h>

#include "config.h"
#include "logger.h"
#include "net.h"
#include "operations.h"
#include "run.h"
#include "run_request.h"
#include "run_response.h"

namespace fs = std::filesystem;

void FillTask(const Task &task, libsbox::Task *libsbox_task) {
    libsbox_task->set_argv(task.argv);
    for (const auto &bind : task.binds) {
        libsbox::BindRule bind_rule(bind.inside, (fs::path(USER_DIR) / bind.outside).string());
        if (bind.allow_write) {
            bind_rule.allow_write();
        }
        if (!bind.allow_exec) {
            bind_rule.forbid_exec();
        }
        libsbox_task->get_binds().push_back(bind_rule);
    }
    libsbox_task->get_env() = task.env;
    // TODO: pipes
    if (task.stdin_.has_value()) {
        libsbox_task->get_stdin().use_file(task.stdin_.value());
    } else {
        libsbox_task->get_stdin().disable();
    }
    if (task.stdout_.has_value()) {
        libsbox_task->get_stdout().use_file(task.stdout_.value());
    } else {
        libsbox_task->get_stdout().disable();
    }
    if (task.stderr_.has_value()) {
        libsbox_task->get_stderr().use_file(task.stderr_.value());
    } else {
        libsbox_task->get_stderr().use_stdout();
    }
    if (task.time_limit_ms.has_value()) {
        libsbox_task->set_time_limit_ms(task.time_limit_ms.value());
    }
    if (task.wall_time_limit_ms.has_value()) {
        libsbox_task->set_wall_time_limit_ms(task.wall_time_limit_ms.value());
    }
    if (task.memory_limit_kb.has_value()) {
        libsbox_task->set_memory_limit_kb(task.memory_limit_kb.value());
    }
    if (task.fsize_limit_kb.has_value()) {
        libsbox_task->set_fsize_limit_kb(task.fsize_limit_kb.value());
    }
    if (task.max_files.has_value()) {
        libsbox_task->set_max_files(task.max_files.value());
    }
    if (task.max_threads.has_value()) {
        libsbox_task->set_max_threads(task.max_threads.value());
    }
}

void FillResult(const libsbox::Task *libsbox_task, Result &result) {
    result.time_usage_ms = libsbox_task->get_time_usage_ms();
    result.time_usage_sys_ms = libsbox_task->get_time_usage_sys_ms();
    result.time_usage_user_ms = libsbox_task->get_time_usage_user_ms();
    result.wall_time_usage_ms = libsbox_task->get_wall_time_usage_ms();
    result.memory_usage_kb = libsbox_task->get_memory_usage_kb();
    result.time_limit_exceeded = libsbox_task->is_time_limit_exceeded();
    result.wall_time_limit_exceeded = libsbox_task->is_wall_time_limit_exceeded();
    result.memory_limit_exceeded = libsbox_task->is_memory_limit_hit();
    result.exited = libsbox_task->exited();
    result.exit_code = libsbox_task->get_exit_code();
    result.signaled = libsbox_task->signaled();
    result.term_signal = libsbox_task->get_term_signal();
    result.oom_killed = libsbox_task->is_oom_killed();
}

void HandleMessage(const std::string &message, WebsocketClientSession &session) {
    RunRequest run_request;
    Load(run_request, ParseJSON(message));
    auto container_path = fs::path(SANDBOX_DIR) / run_request.container;
    std::vector<libsbox::Task *> libsbox_tasks(run_request.tasks.size());
    for (size_t task_id = 0; task_id < libsbox_tasks.size(); ++task_id) {
        libsbox::Task *libsbox_task = libsbox_tasks[task_id] = new libsbox::Task;
        libsbox_task->get_binds().push_back(libsbox::BindRule(".", container_path.string()));
        FillTask(run_request.tasks[task_id], libsbox_task);
    }
    auto error = libsbox::run_together(libsbox_tasks);
    RunResponse run_response;
    if (error) {
        run_response.has_error = true;
        run_response.error = error.get();
    } else {
        run_response.has_error = false;
        for (const auto *libsbox_task : libsbox_tasks) {
            Result result;
            FillResult(libsbox_task, result);
            run_response.results.push_back(result);
        }
    }
    for (const auto *libsbox_task : libsbox_tasks) {
        delete libsbox_task;
    }
    session.Write(StringifyJSON(Dump(run_response)));
}

void Run() {
    Logger::Get().SetName("runner");
    while (true) {
        try {
            WebsocketClientSession session;
            session.Connect(Config::Get().scheduler_host, Config::Get().scheduler_port,
                            "/runner/" + Config::Get().partition);
            Log("Connected to ", Config::Get().scheduler_host, ":", Config::Get().scheduler_port);
            session.OnRead([&](const std::string &message) { HandleMessage(message, session); });
            session.Run();
        } catch (const beast::system_error &error) {
            Log(error.what());
            std::this_thread::sleep_for(
                std::chrono::milliseconds(Config::Get().reconnect_interval_ms));
        }
    }
}
