#include "serialization/all.h"
#include "structures/run_request.h"

template <>
rapidjson::Value Serialize<RunRequest>(const RunRequest &data,
                                       rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("binds", Serialize(data.binds, alloc), alloc);
    value.AddMember("argv", Serialize(data.argv, alloc), alloc);
    value.AddMember("env", Serialize(data.env, alloc), alloc);
    value.AddMember("constraints", Serialize(data.constraints, alloc), alloc);
    return value;
}

template <>
void Deserialize<RunRequest>(RunRequest &data, const rapidjson::Value &value) {
    Deserialize(data.binds, value["binds"]);
    Deserialize(data.argv, value["argv"]);
    Deserialize(data.env, value["env"]);
    Deserialize(data.constraints, value["constraints"]);
}
