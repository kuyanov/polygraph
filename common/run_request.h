#pragma once

#include <vector>

#include "bind.h"
#include "constraints.h"
#include "serialize.h"

struct RunRequest {
    std::vector<Bind> binds;
    std::vector<std::string> argv, env;
    Constraints constraints;
};

template <>
inline rapidjson::Value Serialize<RunRequest>(const RunRequest &data,
                                              rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("binds", Serialize(data.binds, alloc), alloc);
    value.AddMember("argv", Serialize(data.argv, alloc), alloc);
    value.AddMember("env", Serialize(data.env, alloc), alloc);
    value.AddMember("constraints", Serialize(data.constraints, alloc), alloc);
    return value;
}

template <>
inline void Deserialize<RunRequest>(RunRequest &data, const rapidjson::Value &value) {
    Deserialize(data.binds, value["binds"]);
    Deserialize(data.argv, value["argv"]);
    Deserialize(data.env, value["env"]);
    Deserialize(data.constraints, value["constraints"]);
}
