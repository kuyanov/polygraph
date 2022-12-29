#include "constraints.h"
#include "serialize.h"

template <>
rapidjson::Value Serialize<Constraints>(const Constraints &data,
                                        rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
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
void Deserialize<Constraints>(Constraints &data, const rapidjson::Value &value) {
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
