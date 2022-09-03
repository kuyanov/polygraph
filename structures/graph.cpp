#include "graph.h"
#include "operations.h"

template <>
void Load<BlockInput>(BlockInput &object, const rapidjson::Value &json) {
    object.name = json["name"].GetString();
}

template <>
rapidjson::Value Dump<BlockInput>(const BlockInput &object,
                                  rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("name", rapidjson::Value().SetString(object.name.c_str(), alloc), alloc);
    return json;
}

template <>
void Load<BlockOutput>(BlockOutput &object, const rapidjson::Value &json) {
    object.name = json["name"].GetString();
}

template <>
rapidjson::Value Dump<BlockOutput>(const BlockOutput &object,
                                   rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("name", rapidjson::Value().SetString(object.name.c_str(), alloc), alloc);
    return json;
}

template <>
void Load<Block>(Block &object, const rapidjson::Value &json) {
    object.name = json["name"].GetString();
    auto inputs_array = json["inputs"].GetArray();
    object.inputs.resize(inputs_array.Size());
    for (size_t i = 0; i < object.inputs.size(); ++i) {
        Load(object.inputs[i], inputs_array[i]);
    }
    auto outputs_array = json["outputs"].GetArray();
    object.outputs.resize(outputs_array.Size());
    for (size_t i = 0; i < object.outputs.size(); ++i) {
        Load(object.outputs[i], outputs_array[i]);
    }
    auto tasks_array = json["tasks"].GetArray();
    object.tasks.resize(tasks_array.Size());
    for (size_t i = 0; i < object.tasks.size(); ++i) {
        Load(object.tasks[i], tasks_array[i]);
    }
}

template <>
rapidjson::Value Dump<Block>(const Block &object, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("name", rapidjson::Value().SetString(object.name.c_str(), alloc), alloc);
    rapidjson::Value inputs_array(rapidjson::kArrayType);
    for (const auto &input : object.inputs) {
        inputs_array.PushBack(Dump(input, alloc), alloc);
    }
    json.AddMember("inputs", inputs_array, alloc);
    rapidjson::Value outputs_array(rapidjson::kArrayType);
    for (const auto &output : object.outputs) {
        outputs_array.PushBack(Dump(output, alloc), alloc);
    }
    json.AddMember("outputs", outputs_array, alloc);
    rapidjson::Value tasks_array(rapidjson::kArrayType);
    for (const auto &task : object.tasks) {
        tasks_array.PushBack(Dump(task, alloc), alloc);
    }
    json.AddMember("tasks", tasks_array, alloc);
    return json;
}

template <>
void Load<Connection>(Connection &object, const rapidjson::Value &json) {
    object.start_block_id = json["start-block-id"].GetInt();
    object.start_output_id = json["start-output-id"].GetInt();
    object.end_block_id = json["end-block-id"].GetInt();
    object.end_input_id = json["end-input-id"].GetInt();
}

template <>
rapidjson::Value Dump<Connection>(const Connection &object,
                                  rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("start-block-id", rapidjson::Value().SetInt(object.start_block_id), alloc);
    json.AddMember("start-output-id", rapidjson::Value().SetInt(object.start_output_id), alloc);
    json.AddMember("end-block-id", rapidjson::Value().SetInt(object.end_block_id), alloc);
    json.AddMember("end-input-id", rapidjson::Value().SetInt(object.end_input_id), alloc);
    return json;
}

template <>
void Load<Meta>(Meta &object, const rapidjson::Value &json) {
    object.name = json["name"].GetString();
    object.partition = json["partition"].GetString();
    object.max_runners = json["max-runners"].GetInt();
}

template <>
rapidjson::Value Dump<Meta>(const Meta &object, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("name", rapidjson::Value().SetString(object.name.c_str(), alloc), alloc);
    json.AddMember("partition", rapidjson::Value().SetString(object.partition.c_str(), alloc),
                   alloc);
    json.AddMember("max-runners", rapidjson::Value().SetInt(object.max_runners), alloc);
    return json;
}

template <>
void Load<Graph>(Graph &object, const rapidjson::Value &json) {
    auto blocks_array = json["blocks"].GetArray();
    object.blocks.resize(blocks_array.Size());
    for (size_t i = 0; i < object.blocks.size(); ++i) {
        Load(object.blocks[i], blocks_array[i]);
    }
    auto connections_array = json["connections"].GetArray();
    object.connections.resize(connections_array.Size());
    for (size_t i = 0; i < object.connections.size(); ++i) {
        Load(object.connections[i], connections_array[i]);
    }
    Load(object.meta, json["meta"]);
}

template <>
rapidjson::Value Dump<Graph>(const Graph &object, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value json(rapidjson::kObjectType);
    rapidjson::Value blocks_array(rapidjson::kArrayType);
    for (const auto &block : object.blocks) {
        blocks_array.PushBack(Dump(block, alloc), alloc);
    }
    json.AddMember("blocks", blocks_array, alloc);
    rapidjson::Value connections_array(rapidjson::kArrayType);
    for (const auto &connection : object.connections) {
        connections_array.PushBack(Dump(connection, alloc), alloc);
    }
    json.AddMember("connections", connections_array, alloc);
    json.AddMember("meta", Dump(object.meta, alloc), alloc);
    return json;
}
