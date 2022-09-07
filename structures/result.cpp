#include "operations.h"
#include "result.h"

template <>
void Load<Result>(Result &data, const rapidjson::Value &value) {
    Load(data.exited, value["exited"]);
    Load(data.signaled, value["signaled"]);
    Load(data.time_limit_exceeded, value["time-limit-exceeded"]);
    Load(data.wall_time_limit_exceeded, value["wall-time-limit-exceeded"]);
    Load(data.memory_limit_exceeded, value["memory-limit-exceeded"]);
    Load(data.oom_killed, value["oom-killed"]);
    Load(data.exit_code, value["exit-code"]);
    Load(data.term_signal, value["term-signal"]);
    Load(data.time_usage_ms, value["time-usage-ms"]);
    Load(data.time_usage_sys_ms, value["time-usage-sys-ms"]);
    Load(data.time_usage_user_ms, value["time-usage-user-ms"]);
    Load(data.wall_time_usage_ms, value["wall-time-usage-ms"]);
    Load(data.memory_usage_kb, value["memory-usage-kb"]);
}

template <>
rapidjson::Value Dump<Result>(const Result &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("exited", Dump(data.exited, alloc), alloc);
    value.AddMember("signaled", Dump(data.signaled, alloc), alloc);
    value.AddMember("time-limit-exceeded", Dump(data.time_limit_exceeded, alloc), alloc);
    value.AddMember("wall-time-limit-exceeded", Dump(data.wall_time_limit_exceeded, alloc), alloc);
    value.AddMember("memory-limit-exceeded", Dump(data.memory_limit_exceeded, alloc), alloc);
    value.AddMember("oom-killed", Dump(data.oom_killed, alloc), alloc);
    value.AddMember("exit-code", Dump(data.exit_code, alloc), alloc);
    value.AddMember("term-signal", Dump(data.term_signal, alloc), alloc);
    value.AddMember("time-usage-ms", Dump(data.time_usage_ms), alloc);
    value.AddMember("time-usage-sys-ms", Dump(data.time_usage_sys_ms, alloc), alloc);
    value.AddMember("time-usage-user-ms", Dump(data.time_usage_user_ms, alloc), alloc);
    value.AddMember("wall-time-usage-ms", Dump(data.wall_time_usage_ms, alloc), alloc);
    value.AddMember("memory-usage-kb", Dump(data.memory_usage_kb, alloc), alloc);
    return value;
}

template <>
void Load<RunResponse>(RunResponse &data, const rapidjson::Value &value) {
    if (value.HasMember("error")) {
        Load(data.error, value["error"]);
    } else {
        Load(data.result, value["result"]);
    }
}

template <>
rapidjson::Value Dump<RunResponse>(const RunResponse &data,
                                   rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    if (data.error.has_value()) {
        value.AddMember("error", Dump(data.error, alloc), alloc);
    } else {
        value.AddMember("result", Dump(data.result, alloc), alloc);
    }
    return value;
}

template <>
void Load<BlockResponse>(BlockResponse &data, const rapidjson::Value &value) {
    Load(data.block_id, value["block-id"]);
    Load(data.state, value["state"]);
    if (value.HasMember("error")) {
        Load(data.error, value["error"]);
    }
    if (value.HasMember("result")) {
        Load(data.result, value["result"]);
    }
}

template <>
rapidjson::Value Dump<BlockResponse>(const BlockResponse &data,
                                     rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("block-id", Dump(data.block_id, alloc), alloc);
    value.AddMember("state", Dump(data.state, alloc), alloc);
    if (data.error.has_value()) {
        value.AddMember("error", Dump(data.error, alloc), alloc);
    }
    if (data.result.has_value()) {
        value.AddMember("result", Dump(data.result, alloc), alloc);
    }
    return value;
}
