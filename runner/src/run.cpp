#include <chrono>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>
#include <libsbox.h>

#include "config.h"
#include "logger.h"
#include "net.h"
#include "run.h"

namespace fs = std::filesystem;

void FillTask(const rapidjson::Value &task_value, libsbox::Task *task) {
    auto argv_array = task_value["argv"].GetArray();
    std::vector<std::string> argv;
    for (size_t arg_id = 0; arg_id < argv_array.Size(); ++arg_id) {
        argv.push_back(argv_array[arg_id].GetString());
    }
    task->set_argv(argv);
    auto binds_array = task_value["binds"].GetArray();
    for (size_t bind_id = 0; bind_id < binds_array.Size(); ++bind_id) {
        std::string inside = binds_array[bind_id]["inside"].GetString();
        std::string outside = binds_array[bind_id]["outside"].GetString();
        std::string outside_full = (fs::path(USER_DIR) / outside).string();
        libsbox::BindRule bind_rule(inside, outside_full);
        if (binds_array[bind_id]["allow-write"].GetBool()) {
            bind_rule.allow_write();
        }
        if (!binds_array[bind_id]["allow-exec"].GetBool()) {
            bind_rule.forbid_exec();
        }
        task->get_binds().push_back(std::move(bind_rule));
    }
    auto env_array = task_value["env"].GetArray();
    for (size_t env_id = 0; env_id < env_array.Size(); ++env_id) {
        task->get_env().push_back(env_array[env_id].GetString());
    }
    // TODO: pipes
    if (task_value.HasMember("stdin")) {
        task->get_stdin().use_file(task_value["stdin"].GetString());
    } else {
        task->get_stdin().disable();
    }
    if (task_value.HasMember("stdout")) {
        task->get_stdout().use_file(task_value["stdout"].GetString());
    } else {
        task->get_stdout().disable();
    }
    if (task_value.HasMember("stderr")) {
        task->get_stderr().use_file(task_value["stderr"].GetString());
    } else {
        task->get_stderr().disable();
    }
    if (task_value.HasMember("time-limit-ms")) {
        task->set_time_limit_ms(task_value["time-limit-ms"].GetInt64());
    }
    if (task_value.HasMember("wall-time-limit-ms")) {
        task->set_wall_time_limit_ms(task_value["wall-time-limit-ms"].GetInt64());
    }
    if (task_value.HasMember("memory-limit-kb")) {
        task->set_memory_limit_kb(task_value["memory-limit-kb"].GetInt64());
    }
    if (task_value.HasMember("fsize-limit-kb")) {
        task->set_fsize_limit_kb(task_value["fsize-limit-kb"].GetInt64());
    }
    if (task_value.HasMember("max-files")) {
        task->set_max_files(task_value["max-files"].GetInt());
    }
    if (task_value.HasMember("max-threads")) {
        task->set_max_threads(task_value["max-threads"].GetInt());
    }
}

template <class Allocator>
rapidjson::Value BuildStatus(const libsbox::Task *task, Allocator &alloc) {
    rapidjson::Value status_value(rapidjson::kObjectType);
    status_value.AddMember("time-usage-ms", rapidjson::Value().SetInt64(task->get_time_usage_ms()),
                           alloc);
    status_value.AddMember("time-usage-sys-ms",
                           rapidjson::Value().SetInt64(task->get_time_usage_sys_ms()), alloc);
    status_value.AddMember("time-usage-user-ms",
                           rapidjson::Value().SetInt64(task->get_time_usage_user_ms()), alloc);
    status_value.AddMember("wall-time-usage-ms",
                           rapidjson::Value().SetInt64(task->get_wall_time_usage_ms()), alloc);
    status_value.AddMember("memory-usage-kb",
                           rapidjson::Value().SetInt64(task->get_memory_usage_kb()), alloc);
    status_value.AddMember("time-limit-exceeded",
                           rapidjson::Value().SetBool(task->is_time_limit_exceeded()), alloc);
    status_value.AddMember("wall-time-limit-exceeded",
                           rapidjson::Value().SetBool(task->is_wall_time_limit_exceeded()), alloc);
    status_value.AddMember("memory-limit-exceeded",
                           rapidjson::Value().SetBool(task->is_memory_limit_hit()), alloc);
    status_value.AddMember("exited", rapidjson::Value().SetBool(task->exited()), alloc);
    status_value.AddMember("exit-code", rapidjson::Value().SetInt(task->get_exit_code()), alloc);
    status_value.AddMember("signaled", rapidjson::Value().SetBool(task->signaled()), alloc);
    status_value.AddMember("term-signal", rapidjson::Value().SetInt(task->get_term_signal()),
                           alloc);
    status_value.AddMember("oom-killed", rapidjson::Value().SetBool(task->is_oom_killed()), alloc);
    return status_value;
}

void HandleMessage(const std::string &message, WebsocketSession &session) {
    auto tasks_document = ParseJSON(message);
    auto tasks_array = tasks_document["tasks"].GetArray();
    auto container_name = tasks_document["container"].GetString();
    auto container_path = fs::path(SANDBOX_DIR) / container_name;
    std::vector<libsbox::Task *> tasks(tasks_array.Size());
    for (size_t task_id = 0; task_id < tasks.size(); ++task_id) {
        tasks[task_id] = new libsbox::Task;
        tasks[task_id]->get_binds().push_back(libsbox::BindRule(".", container_path.string()));
        FillTask(tasks_array[task_id], tasks[task_id]);
    }
    auto error = libsbox::run_together(tasks);
    rapidjson::Document status_document;
    auto &alloc = status_document.GetAllocator();
    if (error) {
        status_document.AddMember("error", rapidjson::Value().SetString(error.get().c_str(), alloc),
                                  alloc);
    } else {
        rapidjson::Value status_array(rapidjson::kArrayType);
        for (const auto *task : tasks) {
            status_array.PushBack(BuildStatus(task, alloc), alloc);
        }
        status_document.AddMember("tasks", status_array, alloc);
    }
    for (const auto *task : tasks) {
        delete task;
    }
    session.Write(StringifyJSON(status_document));
}

void Run() {
    Logger::Get().SetName("runner");
    while (true) {
        try {
            asio::io_context ioc;
            WebsocketSession session(ioc, Config::Get().scheduler_host,
                                     Config::Get().scheduler_port,
                                     "/runner/" + Config::Get().partition);
            Log("Connected to ", Config::Get().scheduler_host, ":", Config::Get().scheduler_port);
            session.OnRead([&](const std::string &message) { HandleMessage(message, session); });
            ioc.run();
        } catch (const beast::system_error &error) {
            Log(error.what());
            std::this_thread::sleep_for(
                std::chrono::milliseconds(Config::Get().reconnect_interval_ms));
        }
    }
}
