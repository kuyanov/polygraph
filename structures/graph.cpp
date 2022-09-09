#include "graph.h"
#include "operations.h"

template <>
void Load<Bind>(Bind &data, const rapidjson::Value &value) {
    Load(data.inside, value["inside"]);
    Load(data.outside, value["outside"]);
}

template <>
rapidjson::Value Dump<Bind>(const Bind &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("inside", Dump(data.inside, alloc), alloc);
    value.AddMember("outside", Dump(data.outside, alloc), alloc);
    return value;
}

template <>
void Load<Input>(Input &data, const rapidjson::Value &value) {
    Load(data.name, value["name"]);
}

template <>
rapidjson::Value Dump<Input>(const Input &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("name", Dump(data.name, alloc), alloc);
    return value;
}

template <>
void Load<Output>(Output &data, const rapidjson::Value &value) {
    Load(data.name, value["name"]);
}

template <>
rapidjson::Value Dump<Output>(const Output &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("name", Dump(data.name, alloc), alloc);
    return value;
}

template <>
void Load<Block>(Block &data, const rapidjson::Value &value) {
    Load(data.name, value["name"]);
    Load(data.binds, value["binds"]);
    Load(data.inputs, value["inputs"]);
    Load(data.outputs, value["outputs"]);
    Load(data.task, value["task"]);
}

template <>
rapidjson::Value Dump<Block>(const Block &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("name", Dump(data.name, alloc), alloc);
    value.AddMember("binds", Dump(data.binds, alloc), alloc);
    value.AddMember("inputs", Dump(data.inputs, alloc), alloc);
    value.AddMember("outputs", Dump(data.outputs, alloc), alloc);
    value.AddMember("task", Dump(data.task, alloc), alloc);
    return value;
}

template <>
void Load<Connection>(Connection &data, const rapidjson::Value &value) {
    Load(data.start_block_id, value["start-block-id"]);
    Load(data.start_output_id, value["start-output-id"]);
    Load(data.end_block_id, value["end-block-id"]);
    Load(data.end_input_id, value["end-input-id"]);
}

template <>
rapidjson::Value Dump<Connection>(const Connection &data,
                                  rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("start-block-id", Dump(data.start_block_id, alloc), alloc);
    value.AddMember("start-output-id", Dump(data.start_output_id, alloc), alloc);
    value.AddMember("end-block-id", Dump(data.end_block_id, alloc), alloc);
    value.AddMember("end-input-id", Dump(data.end_input_id, alloc), alloc);
    return value;
}

template <>
void Load<Meta>(Meta &data, const rapidjson::Value &value) {
    Load(data.name, value["name"]);
    Load(data.partition, value["partition"]);
    Load(data.max_runners, value["max-runners"]);
}

template <>
rapidjson::Value Dump<Meta>(const Meta &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("name", Dump(data.name, alloc), alloc);
    value.AddMember("partition", Dump(data.partition, alloc), alloc);
    value.AddMember("max-runners", Dump(data.max_runners, alloc), alloc);
    return value;
}

template <>
void Load<Graph>(Graph &data, const rapidjson::Value &value) {
    Load(data.blocks, value["blocks"]);
    Load(data.connections, value["connections"]);
    Load(data.meta, value["meta"]);
}

template <>
rapidjson::Value Dump<Graph>(const Graph &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("blocks", Dump(data.blocks, alloc), alloc);
    value.AddMember("connections", Dump(data.connections, alloc), alloc);
    value.AddMember("meta", Dump(data.meta, alloc), alloc);
    return value;
}
