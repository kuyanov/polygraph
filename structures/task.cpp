#include "task.h"

void Bind::LoadFromValue(const rapidjson::Value &json) {
    inside = json["inside"].GetString();
    outside = json["outside"].GetString();
    allow_write = json["allow-write"].GetBool();
    allow_exec = json["allow-exec"].GetBool();
}

rapidjson::Value Bind::DumpToValue(rapidjson::Document::AllocatorType &alloc) const {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("inside", rapidjson::Value().SetString(inside.c_str(), alloc), alloc);
    json.AddMember("outside", rapidjson::Value().SetString(outside.c_str(), alloc), alloc);
    json.AddMember("allow-write", rapidjson::Value().SetBool(allow_write), alloc);
    json.AddMember("allow-exec", rapidjson::Value().SetBool(allow_exec), alloc);
    return json;
}

void Task::LoadFromValue(const rapidjson::Value &json) {
    auto argv_array = json["argv"].GetArray();
    argv.resize(argv_array.Size());
    for (size_t i = 0; i < argv.size(); ++i) {
        argv[i] = argv_array[i].GetString();
    }
    auto binds_array = json["binds"].GetArray();
    binds.resize(binds_array.Size());
    for (size_t i = 0; i < binds.size(); ++i) {
        binds[i].LoadFromValue(binds_array[i]);
    }
    auto env_array = json["env"].GetArray();
    env.resize(env_array.Size());
    for (size_t i = 0; i < env.size(); ++i) {
        env[i] = env_array[i].GetString();
    }
    if (json.HasMember("stdin")) {
        _stdin = json["stdin"].GetString();
    }
    if (json.HasMember("stdout")) {
        _stdout = json["stdout"].GetString();
    }
    if (json.HasMember("stderr")) {
        _stderr = json["stderr"].GetString();
    }
    if (json.HasMember("time-limit-ms")) {
        time_limit_ms = json["time-limit-ms"].GetInt64();
    }
    if (json.HasMember("wall-time-limit-ms")) {
        wall_time_limit_ms = json["wall-time-limit-ms"].GetInt64();
    }
    if (json.HasMember("memory-limit-kb")) {
        memory_limit_kb = json["memory-limit-kb"].GetInt64();
    }
    if (json.HasMember("fsize-limit-kb")) {
        fsize_limit_kb = json["fsize-limit-kb"].GetInt64();
    }
    if (json.HasMember("max-files")) {
        max_files = json["max-files"].GetInt();
    }
    if (json.HasMember("max-threads")) {
        max_threads = json["max-threads"].GetInt();
    }
}

rapidjson::Value Task::DumpToValue(rapidjson::Document::AllocatorType &alloc) const {
    rapidjson::Value json(rapidjson::kObjectType);
    rapidjson::Value argv_array(rapidjson::kArrayType);
    for (const auto &arg : argv) {
        argv_array.PushBack(rapidjson::Value().SetString(arg.c_str(), alloc), alloc);
    }
    json.AddMember("argv", argv_array, alloc);
    rapidjson::Value binds_array(rapidjson::kArrayType);
    for (const auto &bind : binds) {
        binds_array.PushBack(bind.DumpToValue(alloc), alloc);
    }
    json.AddMember("binds", binds_array, alloc);
    rapidjson::Value env_array(rapidjson::kArrayType);
    for (const auto &var : env) {
        env_array.PushBack(rapidjson::Value().SetString(var.c_str(), alloc), alloc);
    }
    json.AddMember("env", env_array, alloc);
    if (_stdin.has_value()) {
        json.AddMember("stdin", rapidjson::Value().SetString(_stdin.value().c_str(), alloc), alloc);
    }
    if (_stdout.has_value()) {
        json.AddMember("stdout", rapidjson::Value().SetString(_stdout.value().c_str(), alloc),
                       alloc);
    }
    if (_stderr.has_value()) {
        json.AddMember("stderr", rapidjson::Value().SetString(_stderr.value().c_str(), alloc),
                       alloc);
    }
    if (time_limit_ms.has_value()) {
        json.AddMember("time-limit-ms", rapidjson::Value().SetInt64(time_limit_ms.value()), alloc);
    }
    if (wall_time_limit_ms.has_value()) {
        json.AddMember("wall-time-limit-ms",
                       rapidjson::Value().SetInt64(wall_time_limit_ms.value()), alloc);
    }
    if (memory_limit_kb.has_value()) {
        json.AddMember("memory-limit-kb", rapidjson::Value().SetInt64(memory_limit_kb.value()),
                       alloc);
    }
    if (fsize_limit_kb.has_value()) {
        json.AddMember("fsize-limit-kb", rapidjson::Value().SetInt64(fsize_limit_kb.value()),
                       alloc);
    }
    if (max_files.has_value()) {
        json.AddMember("max-files", rapidjson::Value().SetInt(max_files.value()), alloc);
    }
    if (max_threads.has_value()) {
        json.AddMember("max-threads", rapidjson::Value().SetInt(max_threads.value()), alloc);
    }
    return json;
}
