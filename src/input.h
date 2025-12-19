#pragma once

#include <string>

#include "serialize.h"

struct Input {
    std::string path;
    bool cached;
};

template <>
inline rapidjson::Value Serialize<Input>(const Input &data,
                                         rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("path", Serialize(data.path, alloc), alloc);
    value.AddMember("cached", Serialize(data.cached, alloc), alloc);
    return value;
}

template <>
inline void Deserialize<Input>(Input &data, const rapidjson::Value &value) {
    Deserialize(data.path, value["path"]);
    Deserialize(data.cached, value["cached"]);
}
