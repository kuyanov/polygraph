#include "operations.h"
#include "result.h"

template <>
void Load<Result>(Result &object, const rapidjson::Value &json) {
    object.time_usage_ms = json["time-usage-ms"].GetInt64();
    object.time_usage_sys_ms = json["time-usage-sys-ms"].GetInt64();
    object.time_usage_user_ms = json["time-usage-user-ms"].GetInt64();
    object.wall_time_usage_ms = json["wall-time-usage-ms"].GetInt64();
    object.memory_usage_kb = json["memory-usage-kb"].GetInt64();
    object.time_limit_exceeded = json["time-limit-exceeded"].GetBool();
    object.wall_time_limit_exceeded = json["wall-time-limit-exceeded"].GetBool();
    object.memory_limit_exceeded = json["memory-limit-exceeded"].GetBool();
    object.exited = json["exited"].GetBool();
    object.exit_code = json["exit-code"].GetInt();
    object.signaled = json["signaled"].GetBool();
    object.term_signal = json["term-signal"].GetInt();
    object.oom_killed = json["oom-killed"].GetBool();
}

template <>
rapidjson::Value Dump<Result>(const Result &object, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("time-usage-ms", rapidjson::Value().SetInt64(object.time_usage_ms), alloc);
    json.AddMember("time-usage-sys-ms", rapidjson::Value().SetInt64(object.time_usage_sys_ms),
                   alloc);
    json.AddMember("time-usage-user-ms", rapidjson::Value().SetInt64(object.time_usage_user_ms),
                   alloc);
    json.AddMember("wall-time-usage-ms", rapidjson::Value().SetInt64(object.wall_time_usage_ms),
                   alloc);
    json.AddMember("memory-usage-kb", rapidjson::Value().SetInt64(object.memory_usage_kb), alloc);
    json.AddMember("time-limit-exceeded", rapidjson::Value().SetBool(object.time_limit_exceeded),
                   alloc);
    json.AddMember("wall-time-limit-exceeded",
                   rapidjson::Value().SetBool(object.wall_time_limit_exceeded), alloc);
    json.AddMember("memory-limit-exceeded",
                   rapidjson::Value().SetBool(object.memory_limit_exceeded), alloc);
    json.AddMember("exited", rapidjson::Value().SetBool(object.exited), alloc);
    json.AddMember("exit-code", rapidjson::Value().SetInt64(object.exit_code), alloc);
    json.AddMember("signaled", rapidjson::Value().SetBool(object.signaled), alloc);
    json.AddMember("term-signal", rapidjson::Value().SetInt(object.term_signal), alloc);
    json.AddMember("oom-killed", rapidjson::Value().SetBool(object.oom_killed), alloc);
    return json;
}
