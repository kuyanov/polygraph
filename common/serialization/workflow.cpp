#include "serialization/all.h"
#include "structures/workflow.h"

template <>
rapidjson::Value Serialize<Input>(const Input &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("path", Serialize(data.path, alloc), alloc);
    value.AddMember("cached", Serialize(data.cached, alloc), alloc);
    return value;
}

template <>
void Deserialize<Input>(Input &data, const rapidjson::Value &value) {
    Deserialize(data.path, value["path"]);
    Deserialize(data.cached, value["cached"]);
}

template <>
rapidjson::Value Serialize<Output>(const Output &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("path", Serialize(data.path, alloc), alloc);
    return value;
}

template <>
void Deserialize<Output>(Output &data, const rapidjson::Value &value) {
    Deserialize(data.path, value["path"]);
}

template <>
rapidjson::Value Serialize<Block>(const Block &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("name", Serialize(data.name, alloc), alloc);
    value.AddMember("inputs", Serialize(data.inputs, alloc), alloc);
    value.AddMember("outputs", Serialize(data.outputs, alloc), alloc);
    value.AddMember("binds", Serialize(data.binds, alloc), alloc);
    value.AddMember("argv", Serialize(data.argv, alloc), alloc);
    value.AddMember("env", Serialize(data.env, alloc), alloc);
    value.AddMember("constraints", Serialize(data.constraints, alloc), alloc);
    return value;
}

template <>
void Deserialize<Block>(Block &data, const rapidjson::Value &value) {
    Deserialize(data.name, value["name"]);
    Deserialize(data.inputs, value["inputs"]);
    Deserialize(data.outputs, value["outputs"]);
    Deserialize(data.binds, value["binds"]);
    Deserialize(data.argv, value["argv"]);
    Deserialize(data.env, value["env"]);
    Deserialize(data.constraints, value["constraints"]);
}

template <>
rapidjson::Value Serialize<Connection>(const Connection &data,
                                       rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("source-block-id", Serialize(data.source_block_id, alloc), alloc);
    value.AddMember("source-output-id", Serialize(data.source_output_id, alloc), alloc);
    value.AddMember("target-block-id", Serialize(data.target_block_id, alloc), alloc);
    value.AddMember("target-input-id", Serialize(data.target_input_id, alloc), alloc);
    return value;
}

template <>
void Deserialize<Connection>(Connection &data, const rapidjson::Value &value) {
    Deserialize(data.source_block_id, value["source-block-id"]);
    Deserialize(data.source_output_id, value["source-output-id"]);
    Deserialize(data.target_block_id, value["target-block-id"]);
    Deserialize(data.target_input_id, value["target-input-id"]);
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
void Deserialize<Meta>(Meta &data, const rapidjson::Value &value) {
    Deserialize(data.name, value["name"]);
    Deserialize(data.partition, value["partition"]);
    Deserialize(data.max_runners, value["max-runners"]);
}

template <>
rapidjson::Value Serialize<Workflow>(const Workflow &data,
                                     rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("blocks", Serialize(data.blocks, alloc), alloc);
    value.AddMember("connections", Serialize(data.connections, alloc), alloc);
    value.AddMember("meta", Serialize(data.meta, alloc), alloc);
    return value;
}

template <>
void Deserialize<Workflow>(Workflow &data, const rapidjson::Value &value) {
    Deserialize(data.blocks, value["blocks"]);
    Deserialize(data.connections, value["connections"]);
    Deserialize(data.meta, value["meta"]);
}
