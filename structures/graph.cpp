#include "graph.h"

void BlockInput::LoadFromValue(const rapidjson::Value &json) {
    name = json["name"].GetString();
}

rapidjson::Value BlockInput::DumpToValue(rapidjson::Document::AllocatorType &alloc) const {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("name", rapidjson::Value().SetString(name.c_str(), alloc), alloc);
    return json;
}

void BlockOutput::LoadFromValue(const rapidjson::Value &json) {
    name = json["name"].GetString();
}

rapidjson::Value BlockOutput::DumpToValue(rapidjson::Document::AllocatorType &alloc) const {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("name", rapidjson::Value().SetString(name.c_str(), alloc), alloc);
    return json;
}

void Block::LoadFromValue(const rapidjson::Value &json) {
    name = json["name"].GetString();
    auto inputs_array = json["inputs"].GetArray();
    inputs.resize(inputs_array.Size());
    for (size_t i = 0; i < inputs.size(); ++i) {
        inputs[i].LoadFromValue(inputs_array[i]);
    }
    auto outputs_array = json["outputs"].GetArray();
    outputs.resize(outputs_array.Size());
    for (size_t i = 0; i < outputs.size(); ++i) {
        outputs[i].LoadFromValue(outputs_array[i]);
    }
    auto tasks_array = json["tasks"].GetArray();
    tasks.resize(tasks_array.Size());
    for (size_t i = 0; i < tasks.size(); ++i) {
        tasks[i].LoadFromValue(tasks_array[i]);
    }
}

rapidjson::Value Block::DumpToValue(rapidjson::Document::AllocatorType &alloc) const {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("name", rapidjson::Value().SetString(name.c_str(), alloc), alloc);
    rapidjson::Value inputs_array(rapidjson::kArrayType);
    for (const auto &input : inputs) {
        inputs_array.PushBack(input.DumpToValue(alloc), alloc);
    }
    json.AddMember("inputs", inputs_array, alloc);
    rapidjson::Value outputs_array(rapidjson::kArrayType);
    for (const auto &output : outputs) {
        outputs_array.PushBack(output.DumpToValue(alloc), alloc);
    }
    json.AddMember("outputs", outputs_array, alloc);
    rapidjson::Value tasks_array(rapidjson::kArrayType);
    for (const auto &task : tasks) {
        tasks_array.PushBack(task.DumpToValue(alloc), alloc);
    }
    json.AddMember("tasks", tasks_array, alloc);
    return json;
}

void Connection::LoadFromValue(const rapidjson::Value &json) {
    start_block_id = json["start-block-id"].GetInt();
    start_output_id = json["start-output-id"].GetInt();
    end_block_id = json["end-block-id"].GetInt();
    end_input_id = json["end-input-id"].GetInt();
}

rapidjson::Value Connection::DumpToValue(rapidjson::Document::AllocatorType &alloc) const {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("start-block-id", rapidjson::Value().SetInt(start_block_id), alloc);
    json.AddMember("start-output-id", rapidjson::Value().SetInt(start_output_id), alloc);
    json.AddMember("end-block-id", rapidjson::Value().SetInt(end_block_id), alloc);
    json.AddMember("end-input-id", rapidjson::Value().SetInt(end_input_id), alloc);
    return json;
}

void Meta::LoadFromValue(const rapidjson::Value &json) {
    name = json["name"].GetString();
    partition = json["partition"].GetString();
    max_runners = json["max-runners"].GetInt();
}

rapidjson::Value Meta::DumpToValue(rapidjson::Document::AllocatorType &alloc) const {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("name", rapidjson::Value().SetString(name.c_str(), alloc), alloc);
    json.AddMember("partition", rapidjson::Value().SetString(partition.c_str(), alloc), alloc);
    json.AddMember("max-runners", rapidjson::Value().SetInt(max_runners), alloc);
    return json;
}

void Graph::LoadFromValue(const rapidjson::Value &json) {
    auto blocks_array = json["blocks"].GetArray();
    blocks.resize(blocks_array.Size());
    for (size_t i = 0; i < blocks.size(); ++i) {
        blocks[i].LoadFromValue(blocks_array[i]);
    }
    auto connections_array = json["connections"].GetArray();
    connections.resize(connections_array.Size());
    for (size_t i = 0; i < connections.size(); ++i) {
        connections[i].LoadFromValue(connections_array[i]);
    }
    meta.LoadFromValue(json["meta"]);
}

rapidjson::Value Graph::DumpToValue(rapidjson::Document::AllocatorType &alloc) const {
    rapidjson::Value json(rapidjson::kObjectType);
    rapidjson::Value blocks_array(rapidjson::kArrayType);
    for (const auto &block : blocks) {
        blocks_array.PushBack(block.DumpToValue(alloc), alloc);
    }
    json.AddMember("blocks", blocks_array, alloc);
    rapidjson::Value connections_array(rapidjson::kArrayType);
    for (const auto &connection : connections) {
        connections_array.PushBack(connection.DumpToValue(alloc), alloc);
    }
    json.AddMember("connections", connections_array, alloc);
    json.AddMember("meta", meta.DumpToValue(alloc), alloc);
    return json;
}
