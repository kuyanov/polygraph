#include "result.h"
#include "serialize.h"

template <>
void Deserialize<Result>(Result &data, const rapidjson::Value &value) {
    Deserialize(data.exited, value["exited"]);
    Deserialize(data.signaled, value["signaled"]);
    Deserialize(data.time_limit_exceeded, value["time-limit-exceeded"]);
    Deserialize(data.wall_time_limit_exceeded, value["wall-time-limit-exceeded"]);
    Deserialize(data.memory_limit_exceeded, value["memory-limit-exceeded"]);
    Deserialize(data.oom_killed, value["oom-killed"]);
    Deserialize(data.exit_code, value["exit-code"]);
    Deserialize(data.term_signal, value["term-signal"]);
    Deserialize(data.time_usage_ms, value["time-usage-ms"]);
    Deserialize(data.time_usage_sys_ms, value["time-usage-sys-ms"]);
    Deserialize(data.time_usage_user_ms, value["time-usage-user-ms"]);
    Deserialize(data.wall_time_usage_ms, value["wall-time-usage-ms"]);
    Deserialize(data.memory_usage_kb, value["memory-usage-kb"]);
}

template <>
rapidjson::Value Serialize<Result>(const Result &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("exited", Serialize(data.exited, alloc), alloc);
    value.AddMember("signaled", Serialize(data.signaled, alloc), alloc);
    value.AddMember("time-limit-exceeded", Serialize(data.time_limit_exceeded, alloc), alloc);
    value.AddMember("wall-time-limit-exceeded", Serialize(data.wall_time_limit_exceeded, alloc),
                    alloc);
    value.AddMember("memory-limit-exceeded", Serialize(data.memory_limit_exceeded, alloc), alloc);
    value.AddMember("oom-killed", Serialize(data.oom_killed, alloc), alloc);
    value.AddMember("exit-code", Serialize(data.exit_code, alloc), alloc);
    value.AddMember("term-signal", Serialize(data.term_signal, alloc), alloc);
    value.AddMember("time-usage-ms", Serialize(data.time_usage_ms), alloc);
    value.AddMember("time-usage-sys-ms", Serialize(data.time_usage_sys_ms, alloc), alloc);
    value.AddMember("time-usage-user-ms", Serialize(data.time_usage_user_ms, alloc), alloc);
    value.AddMember("wall-time-usage-ms", Serialize(data.wall_time_usage_ms, alloc), alloc);
    value.AddMember("memory-usage-kb", Serialize(data.memory_usage_kb, alloc), alloc);
    return value;
}

template <>
void Deserialize<RunResponse>(RunResponse &data, const rapidjson::Value &value) {
    if (value.HasMember("error")) {
        Deserialize(data.error, value["error"]);
    } else {
        Deserialize(data.result, value["result"]);
    }
}

template <>
rapidjson::Value Serialize<RunResponse>(const RunResponse &data,
                                        rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    if (data.error.has_value()) {
        value.AddMember("error", Serialize(data.error, alloc), alloc);
    } else {
        value.AddMember("result", Serialize(data.result, alloc), alloc);
    }
    return value;
}

template <>
void Deserialize<BlockResponse>(BlockResponse &data, const rapidjson::Value &value) {
    Deserialize(data.block_id, value["block-id"]);
    Deserialize(data.state, value["state"]);
    if (value.HasMember("error")) {
        Deserialize(data.error, value["error"]);
    }
    if (value.HasMember("result")) {
        Deserialize(data.result, value["result"]);
    }
}

template <>
rapidjson::Value Serialize<BlockResponse>(const BlockResponse &data,
                                          rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("block-id", Serialize(data.block_id, alloc), alloc);
    value.AddMember("state", Serialize(data.state, alloc), alloc);
    if (data.error.has_value()) {
        value.AddMember("error", Serialize(data.error, alloc), alloc);
    }
    if (data.result.has_value()) {
        value.AddMember("result", Serialize(data.result, alloc), alloc);
    }
    return value;
}
