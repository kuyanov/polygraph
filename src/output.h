#pragma once

#include <string>

#include "serialize.h"

struct Output {
    std::string path;
};

template <>
inline rapidjson::Value Serialize<Output>(const Output &data,
                                          rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("path", Serialize(data.path, alloc), alloc);
    return value;
}

template <>
inline void Deserialize<Output>(Output &data, const rapidjson::Value &value) {
    Deserialize(data.path, value["path"]);
}
