#include "status.h"

void Status::LoadFromValue(const rapidjson::Value &json) {
    time_usage_ms = json["time-usage-ms"].GetInt64();
    time_usage_sys_ms = json["time-usage-sys-ms"].GetInt64();
    time_usage_user_ms = json["time-usage-user-ms"].GetInt64();
    wall_time_usage_ms = json["wall-time-usage-ms"].GetInt64();
    memory_usage_kb = json["memory-usage-kb"].GetInt64();
    time_limit_exceeded = json["time-limit-exceeded"].GetBool();
    wall_time_limit_exceeded = json["wall-time-limit-exceeded"].GetBool();
    memory_limit_exceeded = json["memory-limit-exceeded"].GetBool();
    exited = json["exited"].GetBool();
    exit_code = json["exit-code"].GetInt();
    signaled = json["signaled"].GetBool();
    term_signal = json["term-signal"].GetInt();
    oom_killed = json["oom-killed"].GetBool();
}

rapidjson::Value Status::DumpToValue(rapidjson::Document::AllocatorType &alloc) const {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("time-usage-ms", rapidjson::Value().SetInt64(time_usage_ms), alloc);
    json.AddMember("time-usage-sys-ms", rapidjson::Value().SetInt64(time_usage_sys_ms), alloc);
    json.AddMember("time-usage-user-ms", rapidjson::Value().SetInt64(time_usage_user_ms), alloc);
    json.AddMember("wall-time-usage-ms", rapidjson::Value().SetInt64(wall_time_usage_ms), alloc);
    json.AddMember("memory-usage-kb", rapidjson::Value().SetInt64(memory_usage_kb), alloc);
    json.AddMember("time-limit-exceeded", rapidjson::Value().SetBool(time_limit_exceeded), alloc);
    json.AddMember("wall-time-limit-exceeded", rapidjson::Value().SetBool(wall_time_limit_exceeded),
                   alloc);
    json.AddMember("memory-limit-exceeded", rapidjson::Value().SetBool(memory_limit_exceeded),
                   alloc);
    json.AddMember("exited", rapidjson::Value().SetBool(exited), alloc);
    json.AddMember("exit-code", rapidjson::Value().SetInt64(exit_code), alloc);
    json.AddMember("signaled", rapidjson::Value().SetBool(signaled), alloc);
    json.AddMember("term-signal", rapidjson::Value().SetInt(term_signal), alloc);
    json.AddMember("oom-killed", rapidjson::Value().SetBool(oom_killed), alloc);
    return json;
}
