#include "graph.h"
#include "serialize.h"

template <>
void Deserialize<Bind>(Bind &data, const rapidjson::Value &value) {
    Deserialize(data.inside, value["inside"]);
    Deserialize(data.outside, value["outside"]);
}

template <>
rapidjson::Value Serialize<Bind>(const Bind &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("inside", Serialize(data.inside, alloc), alloc);
    value.AddMember("outside", Serialize(data.outside, alloc), alloc);
    return value;
}

template <>
void Deserialize<Input>(Input &data, const rapidjson::Value &value) {
    Deserialize(data.name, value["name"]);
}

template <>
rapidjson::Value Serialize<Input>(const Input &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("name", Serialize(data.name, alloc), alloc);
    return value;
}

template <>
void Deserialize<Output>(Output &data, const rapidjson::Value &value) {
    Deserialize(data.name, value["name"]);
}

template <>
rapidjson::Value Serialize<Output>(const Output &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("name", Serialize(data.name, alloc), alloc);
    return value;
}

template <>
void Deserialize<Block>(Block &data, const rapidjson::Value &value) {
    Deserialize(data.name, value["name"]);
    Deserialize(data.binds, value["binds"]);
    Deserialize(data.inputs, value["inputs"]);
    Deserialize(data.outputs, value["outputs"]);
    Deserialize(data.task, value["task"]);
}

template <>
rapidjson::Value Serialize<Block>(const Block &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("name", Serialize(data.name, alloc), alloc);
    value.AddMember("binds", Serialize(data.binds, alloc), alloc);
    value.AddMember("inputs", Serialize(data.inputs, alloc), alloc);
    value.AddMember("outputs", Serialize(data.outputs, alloc), alloc);
    value.AddMember("task", Serialize(data.task, alloc), alloc);
    return value;
}

template <>
void Deserialize<Connection>(Connection &data, const rapidjson::Value &value) {
    Deserialize(data.start_block_id, value["start-block-id"]);
    Deserialize(data.start_output_id, value["start-output-id"]);
    Deserialize(data.end_block_id, value["end-block-id"]);
    Deserialize(data.end_input_id, value["end-input-id"]);
}

template <>
rapidjson::Value Serialize<Connection>(const Connection &data,
                                       rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("start-block-id", Serialize(data.start_block_id, alloc), alloc);
    value.AddMember("start-output-id", Serialize(data.start_output_id, alloc), alloc);
    value.AddMember("end-block-id", Serialize(data.end_block_id, alloc), alloc);
    value.AddMember("end-input-id", Serialize(data.end_input_id, alloc), alloc);
    return value;
}

template <>
void Deserialize<Meta>(Meta &data, const rapidjson::Value &value) {
    Deserialize(data.name, value["name"]);
    Deserialize(data.partition, value["partition"]);
    Deserialize(data.max_runners, value["max-runners"]);
}

template <>
rapidjson::Value Serialize<Meta>(const Meta &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("name", Serialize(data.name, alloc), alloc);
    value.AddMember("partition", Serialize(data.partition, alloc), alloc);
    value.AddMember("max-runners", Serialize(data.max_runners, alloc), alloc);
    return value;
}

template <>
void Deserialize<Graph>(Graph &data, const rapidjson::Value &value) {
    Deserialize(data.blocks, value["blocks"]);
    Deserialize(data.connections, value["connections"]);
    Deserialize(data.meta, value["meta"]);
}

template <>
rapidjson::Value Serialize<Graph>(const Graph &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("blocks", Serialize(data.blocks, alloc), alloc);
    value.AddMember("connections", Serialize(data.connections, alloc), alloc);
    value.AddMember("meta", Serialize(data.meta, alloc), alloc);
    return value;
}
