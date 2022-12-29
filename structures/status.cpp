#include "serialize.h"
#include "status.h"

template <>
rapidjson::Value Serialize<Status>(const Status &data, rapidjson::Document::AllocatorType &alloc) {
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
void Deserialize<Status>(Status &data, const rapidjson::Value &value) {
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
