#include <unordered_map>

#include "graph.h"

std::optional<std::string> GetOptionalString(const rapidjson::Value &value, const char *member) {
    if (value.HasMember(member)) {
        return value[member].GetString();
    }
    return {};
}

Graph::Graph(const rapidjson::Document &graph_document) {
    auto blocks_array = graph_document["blocks"].GetArray();
    blocks.resize(blocks_array.Size());
    std::vector<std::unordered_map<std::string, size_t>> inputs_by_name(blocks.size());
    std::vector<std::unordered_map<std::string, size_t>> outputs_by_name(blocks.size());
    for (size_t block_id = 0; block_id < blocks_array.Size(); ++block_id) {
        auto &block = blocks[block_id];
        block.name = blocks_array[block_id]["name"].GetString();
        auto inputs_array = blocks_array[block_id]["inputs"].GetArray();
        block.inputs.resize(inputs_array.Size());
        for (size_t input_id = 0; input_id < inputs_array.Size(); ++input_id) {
            std::string input_name = inputs_array[input_id]["name"].GetString();
            if (inputs_by_name[block_id].contains(input_name)) {
                throw GraphSemanticError("Duplicated input name");
            }
            inputs_by_name[block_id][input_name] = input_id;
            block.inputs[input_id] = {
                .name = input_name,
                .bind_path = GetOptionalString(inputs_array[input_id], "bind-path")};
        }
        auto outputs_array = blocks_array[block_id]["outputs"].GetArray();
        block.outputs.resize(outputs_array.Size());
        for (size_t output_id = 0; output_id < outputs_array.Size(); ++output_id) {
            std::string output_name = outputs_array[output_id]["name"].GetString();
            if (outputs_by_name[block_id].contains(output_name)) {
                throw GraphSemanticError("Duplicated output name");
            }
            outputs_by_name[block_id][output_name] = output_id;
            block.outputs[output_id] = {
                .name = output_name,
                .bind_path = GetOptionalString(outputs_array[output_id], "bind-path")};
        }
        block.tasks = std::make_shared<rapidjson::Document>();
        block.tasks->CopyFrom(blocks_array[block_id]["tasks"], block.tasks->GetAllocator());
    }
    go.resize(blocks.size());
    auto connections_array = graph_document["connections"].GetArray();
    for (const auto &connection_value : connections_array) {
        int start_block_id = connection_value["start-block-id"].GetInt();
        const char *start_block_output = connection_value["start-block-output"].GetString();
        int end_block_id = connection_value["end-block-id"].GetInt();
        const char *end_block_input = connection_value["end-block-input"].GetString();
        if (start_block_id < 0 || start_block_id >= blocks.size()) {
            throw GraphSemanticError("Invalid connection start block");
        }
        if (end_block_id < 0 || end_block_id >= blocks.size()) {
            throw GraphSemanticError("Invalid connection end block");
        }
        auto start_block_output_iter = outputs_by_name[start_block_id].find(start_block_output);
        if (start_block_output_iter == outputs_by_name[start_block_id].end()) {
            throw GraphSemanticError("Invalid connection start block output");
        }
        auto end_block_input_iter = inputs_by_name[end_block_id].find(end_block_input);
        if (end_block_input_iter == inputs_by_name[end_block_id].end()) {
            throw GraphSemanticError("Invalid connection end block input");
        }
        if (blocks[end_block_id].inputs[end_block_input_iter->second].bind_path) {
            throw GraphSemanticError("Connection end block input cannot have bind path");
        }
        go[start_block_id].emplace_back(start_block_id, start_block_output_iter->second,
                                        end_block_id, end_block_input_iter->second);
    }
    meta.runner_group = graph_document["meta"]["runner-group"].GetString();
    meta.max_runners = graph_document["meta"].HasMember("max-runners")
                           ? graph_document["meta"]["max-runners"].GetInt()
                           : INT_MAX;
}
