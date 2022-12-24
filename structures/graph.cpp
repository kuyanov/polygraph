#include "graph.h"
#include "serialize.h"

template <>
void Deserialize<Bind>(Bind &data, const rapidjson::Value &value) {
    Deserialize(data.inside_filename, value["inside-filename"]);
    Deserialize(data.outside_path, value["outside-path"]);
}

template <>
rapidjson::Value Serialize<Bind>(const Bind &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("inside-filename", Serialize(data.inside_filename, alloc), alloc);
    value.AddMember("outside-path", Serialize(data.outside_path, alloc), alloc);
    return value;
}

template <>
void Deserialize<Block>(Block &data, const rapidjson::Value &value) {
    Deserialize(data.name, value["name"]);
    Deserialize(data.binds, value["binds"]);
    Deserialize(data.task, value["task"]);
}

template <>
rapidjson::Value Serialize<Block>(const Block &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("name", Serialize(data.name, alloc), alloc);
    value.AddMember("binds", Serialize(data.binds, alloc), alloc);
    value.AddMember("task", Serialize(data.task, alloc), alloc);
    return value;
}

template <>
void Deserialize<Connection>(Connection &data, const rapidjson::Value &value) {
    Deserialize(data.type, value["type"]);
    Deserialize(data.start_block_id, value["start-block-id"]);
    Deserialize(data.start_filename, value["start-filename"]);
    Deserialize(data.end_block_id, value["end-block-id"]);
    Deserialize(data.end_filename, value["end-filename"]);
}

template <>
rapidjson::Value Serialize<Connection>(const Connection &data,
                                       rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("type", Serialize(data.type, alloc), alloc);
    value.AddMember("start-block-id", Serialize(data.start_block_id, alloc), alloc);
    value.AddMember("start-filename", Serialize(data.start_filename, alloc), alloc);
    value.AddMember("end-block-id", Serialize(data.end_block_id, alloc), alloc);
    value.AddMember("end-filename", Serialize(data.end_filename, alloc), alloc);
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
