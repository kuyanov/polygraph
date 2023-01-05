#include "serialization/all.h"
#include "structures/run_response.h"

template <>
rapidjson::Value Serialize<RunResponse>(const RunResponse &data,
                                        rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    if (data.error.has_value()) {
        value.AddMember("error", Serialize(data.error, alloc), alloc);
    }
    if (data.status.has_value()) {
        value.AddMember("status", Serialize(data.status, alloc), alloc);
    }
    return value;
}

template <>
void Deserialize<RunResponse>(RunResponse &data, const rapidjson::Value &value) {
    if (value.HasMember("error")) {
        Deserialize(data.error, value["error"]);
    }
    if (value.HasMember("status")) {
        Deserialize(data.status, value["status"]);
    }
}
