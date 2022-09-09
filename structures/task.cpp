#include "operations.h"
#include "task.h"

template <>
void Load<Limits>(Limits &data, const rapidjson::Value &value) {
    if (value.HasMember("time-limit-ms")) {
        Load(data.time_limit_ms, value["time-limit-ms"]);
    }
    if (value.HasMember("wall-time-limit-ms")) {
        Load(data.wall_time_limit_ms, value["wall-time-limit-ms"]);
    }
    if (value.HasMember("memory-limit-kb")) {
        Load(data.memory_limit_kb, value["memory-limit-kb"]);
    }
    if (value.HasMember("fsize-limit-kb")) {
        Load(data.fsize_limit_kb, value["fsize-limit-kb"]);
    }
    if (value.HasMember("max-files")) {
        Load(data.max_files, value["max-files"]);
    }
    if (value.HasMember("max-threads")) {
        Load(data.max_threads, value["max-threads"]);
    }
}

template <>
rapidjson::Value Dump<Limits>(const Limits &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    if (data.time_limit_ms.has_value()) {
        value.AddMember("time-limit-ms", Dump(data.time_limit_ms, alloc), alloc);
    }
    if (data.wall_time_limit_ms.has_value()) {
        value.AddMember("wall-time-limit-ms", Dump(data.wall_time_limit_ms, alloc), alloc);
    }
    if (data.memory_limit_kb.has_value()) {
        value.AddMember("memory-limit-kb", Dump(data.memory_limit_kb, alloc), alloc);
    }
    if (data.fsize_limit_kb.has_value()) {
        value.AddMember("fsize-limit-kb", Dump(data.fsize_limit_kb, alloc), alloc);
    }
    if (data.max_files.has_value()) {
        value.AddMember("max-files", Dump(data.max_files, alloc), alloc);
    }
    if (data.max_threads.has_value()) {
        value.AddMember("max-threads", Dump(data.max_threads, alloc), alloc);
    }
    return value;
}

template <>
void Load<Task>(Task &data, const rapidjson::Value &value) {
    Load(data.argv, value["argv"]);
    Load(data.env, value["env"]);
    Load(data.limits, value["limits"]);
}

template <>
rapidjson::Value Dump<Task>(const Task &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("argv", Dump(data.argv, alloc), alloc);
    value.AddMember("env", Dump(data.env, alloc), alloc);
    value.AddMember("limits", Dump(data.limits, alloc), alloc);
    return value;
}

template <>
void Load<RunRequest>(RunRequest &data, const rapidjson::Value &value) {
    Load(data.container, value["container"]);
    Load(data.task, value["task"]);
}

template <>
rapidjson::Value Dump<RunRequest>(const RunRequest &data,
                                  rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("container", Dump(data.container, alloc), alloc);
    value.AddMember("task", Dump(data.task, alloc), alloc);
    return value;
}
