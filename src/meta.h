#pragma once

#include <string>

#include "serialize.h"

struct Meta {
    std::string name, partition;
    int max_runners;
};

template <>
inline rapidjson::Value Serialize<Meta>(const Meta &data,
                                        rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("name", Serialize(data.name, alloc), alloc);
    value.AddMember("partition", Serialize(data.partition, alloc), alloc);
    value.AddMember("max-runners", Serialize(data.max_runners, alloc), alloc);
    return value;
}

template <>
inline void Deserialize<Meta>(Meta &data, const rapidjson::Value &value) {
    Deserialize(data.name, value["name"]);
    Deserialize(data.partition, value["partition"]);
    Deserialize(data.max_runners, value["max-runners"]);
}
