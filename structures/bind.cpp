#include "bind.h"
#include "serialize.h"

template <>
rapidjson::Value Serialize<Bind>(const Bind &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("inside", Serialize(data.inside, alloc), alloc);
    value.AddMember("outside", Serialize(data.outside, alloc), alloc);
    value.AddMember("readonly", Serialize(data.readonly, alloc), alloc);
    return value;
}

template <>
void Deserialize<Bind>(Bind &data, const rapidjson::Value &value) {
    Deserialize(data.inside, value["inside"]);
    Deserialize(data.outside, value["outside"]);
    Deserialize(data.readonly, value["readonly"]);
}
