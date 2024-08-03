#include "serialization/all.h"
#include "structures/submit_response.h"

template <>
rapidjson::Value Serialize<SubmitResponse>(const SubmitResponse &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("status", Serialize(data.status, alloc), alloc);
    value.AddMember("data", Serialize(data.data, alloc), alloc);
    return value;
}

template <>
void Deserialize<SubmitResponse>(SubmitResponse &data, const rapidjson::Value &value) {
    Deserialize(data.status, value["status"]);
    Deserialize(data.data, value["data"]);
}
