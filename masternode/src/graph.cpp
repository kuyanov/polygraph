#include <unordered_set>

#include "constants.h"
#include "error.h"
#include "graph.h"

template <class JsonArray>
void CheckAndFillBlocks(JsonArray blocks_array, std::vector<Graph::Block> &blocks) {
    blocks.resize(blocks_array.Size());
    for (size_t block_id = 0; block_id < blocks_array.Size(); ++block_id) {
        auto &block = blocks[block_id];
        std::unordered_set<std::string> names_set;
        auto inputs_array = blocks_array[block_id]["inputs"].GetArray();
        block.inputs.resize(inputs_array.Size());
        for (size_t input_id = 0; input_id < inputs_array.Size(); ++input_id) {
            std::string input_name = inputs_array[input_id]["name"].GetString();
            names_set.insert(input_name);
            block.inputs[input_id] = {.name = input_name};
        }
        auto outputs_array = blocks_array[block_id]["outputs"].GetArray();
        block.outputs.resize(outputs_array.Size());
        for (size_t output_id = 0; output_id < outputs_array.Size(); ++output_id) {
            std::string output_name = outputs_array[output_id]["name"].GetString();
            names_set.insert(output_name);
            block.outputs[output_id] = {.name = output_name};
        }
        auto externals_array = blocks_array[block_id]["externals"].GetArray();
        block.externals.resize(externals_array.Size());
        for (size_t external_id = 0; external_id < externals_array.Size(); ++external_id) {
            std::string external_name = externals_array[external_id]["name"].GetString();
            std::string user_path = externals_array[external_id]["user-path"].GetString();
            names_set.insert(external_name);
            block.externals[external_id] = {.name = external_name, .user_path = user_path};
        }
        if (names_set.contains("")) {
            throw SemanticError(errors::kEmptyFilename);
        }
        if (names_set.size() !=
            block.inputs.size() + block.outputs.size() + block.externals.size()) {
            throw SemanticError(errors::kDuplicatedFilename);
        }
        block.tasks.CopyFrom(blocks_array[block_id]["tasks"], block.tasks.GetAllocator());
    }
}

template <class JsonArray>
void CheckAndFillConnections(JsonArray connections_array,
                             std::vector<std::vector<Graph::Connection>> &connections,
                             const std::vector<Graph::Block> &blocks) {
    connections.resize(blocks.size());
    for (const auto &connection_value : connections_array) {
        int start_block_id = connection_value["start-block-id"].GetInt();
        int start_output_id = connection_value["start-output-id"].GetInt();
        int end_block_id = connection_value["end-block-id"].GetInt();
        int end_input_id = connection_value["end-input-id"].GetInt();
        if (start_block_id < 0 || start_block_id >= blocks.size()) {
            throw SemanticError(errors::kInvalidStartBlock);
        }
        if (start_output_id < 0 || start_output_id >= blocks[start_block_id].outputs.size()) {
            throw SemanticError(errors::kInvalidStartBlockOutput);
        }
        if (end_block_id < 0 || end_block_id >= blocks.size()) {
            throw SemanticError(errors::kInvalidEndBlock);
        }
        if (end_input_id < 0 || end_input_id >= blocks[end_block_id].inputs.size()) {
            throw SemanticError(errors::kInvalidEndBlockInput);
        }
        if (start_block_id == end_block_id) {
            throw SemanticError(errors::kLoopsNotSupported);
        }
        connections[start_block_id].emplace_back(start_block_id, start_output_id, end_block_id,
                                                 end_input_id);
    }
}

template <class JsonObject>
void CheckAndFillMeta(JsonObject meta_object, Graph::Meta &meta) {
    meta.name = meta_object["name"].GetString();
    meta.runner_group = meta_object["runner-group"].GetString();
    meta.max_runners = meta_object["max-runners"].GetInt();
    if (meta.max_runners < 1) {
        throw SemanticError(errors::kMaxRunnersAtLeast1);
    }
}

Graph::Graph(const rapidjson::Document &graph_document) {
    CheckAndFillBlocks(graph_document["blocks"].GetArray(), blocks);
    CheckAndFillConnections(graph_document["connections"].GetArray(), connections, blocks);
    CheckAndFillMeta(graph_document["meta"].GetObject(), meta);
}
