#pragma once

#include <string>
#include <vector>

#include "bind.h"
#include "constraints.h"
#include "input.h"
#include "output.h"
#include "serialize.h"

struct Block {
    std::string name;
    std::vector<Input> inputs;
    std::vector<Output> outputs;
    std::vector<Bind> binds;
    std::vector<std::string> argv, env;
    Constraints constraints;
};

template <>
inline rapidjson::Value Serialize<Block>(const Block &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("name", Serialize(data.name, alloc), alloc);
    value.AddMember("inputs", Serialize(data.inputs, alloc), alloc);
    value.AddMember("outputs", Serialize(data.outputs, alloc), alloc);
    value.AddMember("binds", Serialize(data.binds, alloc), alloc);
    value.AddMember("argv", Serialize(data.argv, alloc), alloc);
    value.AddMember("env", Serialize(data.env, alloc), alloc);
    value.AddMember("constraints", Serialize(data.constraints, alloc), alloc);
    return value;
}

template <>
inline void Deserialize<Block>(Block &data, const rapidjson::Value &value) {
    Deserialize(data.name, value["name"]);
    Deserialize(data.inputs, value["inputs"]);
    Deserialize(data.outputs, value["outputs"]);
    Deserialize(data.binds, value["binds"]);
    Deserialize(data.argv, value["argv"]);
    Deserialize(data.env, value["env"]);
    Deserialize(data.constraints, value["constraints"]);
}
