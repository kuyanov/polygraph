#include "serialize.h"
#include "task.h"

template <>
void Deserialize<Task>(Task &data, const rapidjson::Value &value) {
    Deserialize(data.argv, value["argv"]);
    Deserialize(data.env, value["env"]);
    if (value.HasMember("time-limit-ms")) {
        Deserialize(data.time_limit_ms, value["time-limit-ms"]);
    }
    if (value.HasMember("wall-time-limit-ms")) {
        Deserialize(data.wall_time_limit_ms, value["wall-time-limit-ms"]);
    }
    if (value.HasMember("memory-limit-kb")) {
        Deserialize(data.memory_limit_kb, value["memory-limit-kb"]);
    }
    if (value.HasMember("fsize-limit-kb")) {
        Deserialize(data.fsize_limit_kb, value["fsize-limit-kb"]);
    }
    if (value.HasMember("max-files")) {
        Deserialize(data.max_files, value["max-files"]);
    }
    if (value.HasMember("max-threads")) {
        Deserialize(data.max_threads, value["max-threads"]);
    }
}

template <>
rapidjson::Value Serialize<Task>(const Task &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("argv", Serialize(data.argv, alloc), alloc);
    value.AddMember("env", Serialize(data.env, alloc), alloc);
    if (data.time_limit_ms.has_value()) {
        value.AddMember("time-limit-ms", Serialize(data.time_limit_ms, alloc), alloc);
    }
    if (data.wall_time_limit_ms.has_value()) {
        value.AddMember("wall-time-limit-ms", Serialize(data.wall_time_limit_ms, alloc), alloc);
    }
    if (data.memory_limit_kb.has_value()) {
        value.AddMember("memory-limit-kb", Serialize(data.memory_limit_kb, alloc), alloc);
    }
    if (data.fsize_limit_kb.has_value()) {
        value.AddMember("fsize-limit-kb", Serialize(data.fsize_limit_kb, alloc), alloc);
    }
    if (data.max_files.has_value()) {
        value.AddMember("max-files", Serialize(data.max_files, alloc), alloc);
    }
    if (data.max_threads.has_value()) {
        value.AddMember("max-threads", Serialize(data.max_threads, alloc), alloc);
    }
    return value;
}

template <>
void Deserialize<RunRequest>(RunRequest &data, const rapidjson::Value &value) {
    Deserialize(data.container, value["container"]);
    Deserialize(data.task, value["task"]);
}

template <>
rapidjson::Value Serialize<RunRequest>(const RunRequest &data,
                                       rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("container", Serialize(data.container, alloc), alloc);
    value.AddMember("task", Serialize(data.task, alloc), alloc);
    return value;
}
