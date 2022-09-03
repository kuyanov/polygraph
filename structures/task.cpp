#include "operations.h"
#include "task.h"

template <>
void Load<Bind>(Bind &object, const rapidjson::Value &json) {
    object.inside = json["inside"].GetString();
    object.outside = json["outside"].GetString();
    object.allow_write = json["allow-write"].GetBool();
    object.allow_exec = json["allow-exec"].GetBool();
}

template <>
rapidjson::Value Dump<Bind>(const Bind &object, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("inside", rapidjson::Value().SetString(object.inside.c_str(), alloc), alloc);
    json.AddMember("outside", rapidjson::Value().SetString(object.outside.c_str(), alloc), alloc);
    json.AddMember("allow-write", rapidjson::Value().SetBool(object.allow_write), alloc);
    json.AddMember("allow-exec", rapidjson::Value().SetBool(object.allow_exec), alloc);
    return json;
}

template <>
void Load<Task>(Task &object, const rapidjson::Value &json) {
    auto argv_array = json["argv"].GetArray();
    object.argv.resize(argv_array.Size());
    for (size_t i = 0; i < object.argv.size(); ++i) {
        object.argv[i] = argv_array[i].GetString();
    }
    auto binds_array = json["binds"].GetArray();
    object.binds.resize(binds_array.Size());
    for (size_t i = 0; i < object.binds.size(); ++i) {
        Load(object.binds[i], binds_array[i]);
    }
    auto env_array = json["env"].GetArray();
    object.env.resize(env_array.Size());
    for (size_t i = 0; i < object.env.size(); ++i) {
        object.env[i] = env_array[i].GetString();
    }
    if (json.HasMember("stdin")) {
        object.stdin_ = json["stdin"].GetString();
    }
    if (json.HasMember("stdout")) {
        object.stdout_ = json["stdout"].GetString();
    }
    if (json.HasMember("stderr")) {
        object.stderr_ = json["stderr"].GetString();
    }
    if (json.HasMember("time-limit-ms")) {
        object.time_limit_ms = json["time-limit-ms"].GetInt64();
    }
    if (json.HasMember("wall-time-limit-ms")) {
        object.wall_time_limit_ms = json["wall-time-limit-ms"].GetInt64();
    }
    if (json.HasMember("memory-limit-kb")) {
        object.memory_limit_kb = json["memory-limit-kb"].GetInt64();
    }
    if (json.HasMember("fsize-limit-kb")) {
        object.fsize_limit_kb = json["fsize-limit-kb"].GetInt64();
    }
    if (json.HasMember("max-files")) {
        object.max_files = json["max-files"].GetInt();
    }
    if (json.HasMember("max-threads")) {
        object.max_threads = json["max-threads"].GetInt();
    }
}

template <>
rapidjson::Value Dump<Task>(const Task &object, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value json(rapidjson::kObjectType);
    rapidjson::Value argv_array(rapidjson::kArrayType);
    for (const auto &arg : object.argv) {
        argv_array.PushBack(rapidjson::Value().SetString(arg.c_str(), alloc), alloc);
    }
    json.AddMember("argv", argv_array, alloc);
    rapidjson::Value binds_array(rapidjson::kArrayType);
    for (const auto &bind : object.binds) {
        binds_array.PushBack(Dump(bind, alloc), alloc);
    }
    json.AddMember("binds", binds_array, alloc);
    rapidjson::Value env_array(rapidjson::kArrayType);
    for (const auto &var : object.env) {
        env_array.PushBack(rapidjson::Value().SetString(var.c_str(), alloc), alloc);
    }
    json.AddMember("env", env_array, alloc);
    if (object.stdin_.has_value()) {
        json.AddMember("stdin", rapidjson::Value().SetString(object.stdin_.value().c_str(), alloc),
                       alloc);
    }
    if (object.stdout_.has_value()) {
        json.AddMember("stdout",
                       rapidjson::Value().SetString(object.stdout_.value().c_str(), alloc), alloc);
    }
    if (object.stderr_.has_value()) {
        json.AddMember("stderr",
                       rapidjson::Value().SetString(object.stderr_.value().c_str(), alloc), alloc);
    }
    if (object.time_limit_ms.has_value()) {
        json.AddMember("time-limit-ms", rapidjson::Value().SetInt64(object.time_limit_ms.value()),
                       alloc);
    }
    if (object.wall_time_limit_ms.has_value()) {
        json.AddMember("wall-time-limit-ms",
                       rapidjson::Value().SetInt64(object.wall_time_limit_ms.value()), alloc);
    }
    if (object.memory_limit_kb.has_value()) {
        json.AddMember("memory-limit-kb",
                       rapidjson::Value().SetInt64(object.memory_limit_kb.value()), alloc);
    }
    if (object.fsize_limit_kb.has_value()) {
        json.AddMember("fsize-limit-kb", rapidjson::Value().SetInt64(object.fsize_limit_kb.value()),
                       alloc);
    }
    if (object.max_files.has_value()) {
        json.AddMember("max-files", rapidjson::Value().SetInt(object.max_files.value()), alloc);
    }
    if (object.max_threads.has_value()) {
        json.AddMember("max-threads", rapidjson::Value().SetInt(object.max_threads.value()), alloc);
    }
    return json;
}
