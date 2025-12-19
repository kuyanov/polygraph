#pragma once

#include <cstdint>

#include "serialize.h"

struct Connection {
    size_t source_block_id, source_output_id, target_block_id, target_input_id;
};

template <>
inline rapidjson::Value Serialize<Connection>(const Connection &data,
                                              rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("source-block-id", Serialize(data.source_block_id, alloc), alloc);
    value.AddMember("source-output-id", Serialize(data.source_output_id, alloc), alloc);
    value.AddMember("target-block-id", Serialize(data.target_block_id, alloc), alloc);
    value.AddMember("target-input-id", Serialize(data.target_input_id, alloc), alloc);
    return value;
}

template <>
inline void Deserialize<Connection>(Connection &data, const rapidjson::Value &value) {
    Deserialize(data.source_block_id, value["source-block-id"]);
    Deserialize(data.source_output_id, value["source-output-id"]);
    Deserialize(data.target_block_id, value["target-block-id"]);
    Deserialize(data.target_input_id, value["target-input-id"]);
}
