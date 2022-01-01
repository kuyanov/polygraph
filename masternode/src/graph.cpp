#include "graph.h"

std::optional<std::string> GetOptionalString(const rapidjson::Value &value, const char *member) {
    if (value.HasMember(member)) {
        return value[member].GetString();
    }
    return {};
}

Graph::Graph(const rapidjson::Document &graph_document) {
    auto blocks_array = graph_document["blocks"].GetArray();
    blocks.reserve(blocks_array.Size());
    for (const auto &block_value : blocks_array) {
        auto &block = blocks.emplace_back();
        block.name = block_value["name"].GetString();
        auto inputs_array = block_value["inputs"].GetArray();
        block.inputs.reserve(inputs_array.Size());
        for (const auto &input_value : inputs_array) {
            std::string input_name = input_value["name"].GetString();
            block.inputs.emplace_back(input_name, GetOptionalString(input_value, "bind-path"));
            if (block.inputs_by_name.contains(input_name)) {
                throw GraphSemanticError("Duplicated input name");
            }
            block.inputs_by_name[input_name] = &block.inputs.back();
        }
        auto outputs_array = block_value["outputs"].GetArray();
        block.outputs.reserve(outputs_array.Size());
        for (const auto &output_value : outputs_array) {
            std::string output_name = output_value["name"].GetString();
            block.outputs.emplace_back(output_name, GetOptionalString(output_value, "bind-path"));
            if (block.outputs_by_name.contains(output_name)) {
                throw GraphSemanticError("Duplicated output name");
            }
            block.outputs_by_name[output_name] = &block.outputs.back();
        }
        block.tasks = std::make_shared<rapidjson::Document>();
        block.tasks->CopyFrom(block_value["tasks"], block.tasks->GetAllocator());
    }
    go.resize(blocks.size());
    auto connections_array = graph_document["connections"].GetArray();
    for (const auto &connection_value : connections_array) {
        int start_block_id = connection_value["start-block-id"].GetInt();
        std::string start_block_output = connection_value["start-block-output"].GetString();
        int end_block_id = connection_value["end-block-id"].GetInt();
        std::string end_block_input = connection_value["end-block-input"].GetString();
        if (start_block_id < 0 || start_block_id >= blocks.size()) {
            throw GraphSemanticError("Invalid connection start block");
        }
        if (end_block_id < 0 || end_block_id >= blocks.size()) {
            throw GraphSemanticError("Invalid connection end block");
        }
        if (!blocks[start_block_id].outputs_by_name.contains(start_block_output)) {
            throw GraphSemanticError("Invalid connection start block output");
        }
        if (!blocks[end_block_id].inputs_by_name.contains(end_block_input)) {
            throw GraphSemanticError("Invalid connection end block input");
        }
        go[start_block_id].emplace_back(start_block_id, start_block_output, end_block_id,
                                        end_block_input);
    }
    meta.runner_group = graph_document["meta"]["runner-group"].GetString();
    meta.max_runners = graph_document["meta"].HasMember("max-runners")
                           ? graph_document["meta"]["max-runners"].GetInt()
                           : INT_MAX;
}
