#include <unordered_set>

#include "error.h"
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
    std::unordered_set<std::string> names_set;
    for (size_t block_id = 0; block_id < blocks_array.Size(); ++block_id) {
        auto &block = blocks[block_id];
        block.name = blocks_array[block_id]["name"].GetString();
        auto inputs_array = blocks_array[block_id]["inputs"].GetArray();
        block.inputs.resize(inputs_array.Size());
        for (size_t input_id = 0; input_id < inputs_array.Size(); ++input_id) {
            std::string input_name = inputs_array[input_id]["name"].GetString();
            names_set.insert(input_name);
            block.inputs[input_id] = {
                .name = input_name,
                .bind_path = GetOptionalString(inputs_array[input_id], "bind-path")};
        }
        if (names_set.size() != inputs_array.Size()) {
            throw SemanticError("Duplicated input name");
        }
        names_set.clear();
        auto outputs_array = blocks_array[block_id]["outputs"].GetArray();
        block.outputs.resize(outputs_array.Size());
        for (size_t output_id = 0; output_id < outputs_array.Size(); ++output_id) {
            std::string output_name = outputs_array[output_id]["name"].GetString();
            names_set.insert(output_name);
            block.outputs[output_id] = {
                .name = output_name,
                .bind_path = GetOptionalString(outputs_array[output_id], "bind-path")};
        }
        if (names_set.size() != outputs_array.Size()) {
            throw SemanticError("Duplicated output name");
        }
        names_set.clear();
        block.tasks.CopyFrom(blocks_array[block_id]["tasks"], block.tasks.GetAllocator());
    }
    go.resize(blocks.size());
    auto connections_array = graph_document["connections"].GetArray();
    for (const auto &connection_value : connections_array) {
        int start_block_id = connection_value["start-block-id"].GetInt();
        int start_output_id = connection_value["start-output-id"].GetInt();
        int end_block_id = connection_value["end-block-id"].GetInt();
        int end_input_id = connection_value["end-input-id"].GetInt();
        if (start_block_id < 0 || start_block_id >= blocks.size()) {
            throw SemanticError("Invalid connection start block");
        }
        if (start_output_id < 0 || start_output_id >= blocks[start_block_id].outputs.size()) {
            throw SemanticError("Invalid connection start block output");
        }
        if (end_block_id < 0 || end_block_id >= blocks.size()) {
            throw SemanticError("Invalid connection end block");
        }
        if (end_input_id < 0 || end_input_id >= blocks[end_block_id].inputs.size()) {
            throw SemanticError("Invalid connection end block input");
        }
        if (blocks[end_block_id].inputs[end_input_id].bind_path) {
            throw SemanticError("Connection end block input cannot have bind path");
        }
        go[start_block_id].emplace_back(start_block_id, start_output_id, end_block_id,
                                        end_input_id);
    }
    meta.runner_group = graph_document["meta"]["runner-group"].GetString();
    meta.max_runners = graph_document["meta"].HasMember("max-runners")
                           ? graph_document["meta"]["max-runners"].GetInt()
                           : INT_MAX;
    if (meta.max_runners <= 0) {
        throw SemanticError("max-runners must be at least 1");
    }
}
