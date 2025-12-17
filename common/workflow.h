#pragma once

#include <vector>

#include "block.h"
#include "connection.h"
#include "meta.h"
#include "serialize.h"

struct Workflow {
    std::vector<Block> blocks;
    std::vector<Connection> connections;
    Meta meta;
};

template <>
inline rapidjson::Value Serialize<Workflow>(const Workflow &data,
                                            rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("blocks", Serialize(data.blocks, alloc), alloc);
    value.AddMember("connections", Serialize(data.connections, alloc), alloc);
    value.AddMember("meta", Serialize(data.meta, alloc), alloc);
    return value;
}

template <>
inline void Deserialize<Workflow>(Workflow &data, const rapidjson::Value &value) {
    Deserialize(data.blocks, value["blocks"]);
    Deserialize(data.connections, value["connections"]);
    Deserialize(data.meta, value["meta"]);
}
