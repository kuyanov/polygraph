#include "block_response.h"
#include "serialize.h"

template <>
rapidjson::Value Serialize<BlockResponse>(const BlockResponse &data,
                                          rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("block-id", Serialize(data.block_id, alloc), alloc);
    value.AddMember("state", Serialize(data.state, alloc), alloc);
    if (data.error.has_value()) {
        value.AddMember("error", Serialize(data.error, alloc), alloc);
    }
    if (data.status.has_value()) {
        value.AddMember("status", Serialize(data.status, alloc), alloc);
    }
    return value;
}

template <>
void Deserialize<BlockResponse>(BlockResponse &data, const rapidjson::Value &value) {
    Deserialize(data.block_id, value["block-id"]);
    Deserialize(data.state, value["state"]);
    if (value.HasMember("error")) {
        Deserialize(data.error, value["error"]);
    }
    if (value.HasMember("status")) {
        Deserialize(data.status, value["status"]);
    }
}
