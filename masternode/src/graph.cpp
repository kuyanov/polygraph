#include <unordered_set>

#include "constants.h"
#include "error.h"
#include "graph.h"

template <class JsonArray, class BlockIO>
void CheckAndFillIOs(JsonArray ios_array, std::vector<BlockIO> &block_ios,
                     std::unordered_set<std::string> &io_names) {
    block_ios.resize(ios_array.Size());
    for (size_t io_id = 0; io_id < ios_array.Size(); ++io_id) {
        std::string io_name = ios_array[io_id]["name"].GetString();
        if (io_names.contains(io_name)) {
            throw SemanticError(errors::kDuplicatedIOName);
        }
        io_names.insert(io_name);
        block_ios[io_id] = {.name = io_name};
    }
}

template <class JsonArray>
void CheckAndFillExternals(JsonArray externals_array,
                           std::vector<Graph::BlockExternal> &block_externals,
                           const std::unordered_set<std::string> &input_names,
                           const std::unordered_set<std::string> &output_names) {
    block_externals.resize(externals_array.Size());
    std::unordered_set<std::string> external_names;
    for (size_t external_id = 0; external_id < externals_array.Size(); ++external_id) {
        std::string external_name = externals_array[external_id]["name"].GetString();
        std::string user_path = externals_array[external_id]["user-path"].GetString();
        if (input_names.contains(external_name) || output_names.contains(external_name) ||
            external_names.contains(external_name)) {
            throw SemanticError(errors::kDuplicatedExternalName);
        }
        external_names.insert(external_name);
        block_externals[external_id] = {.name = external_name, .user_path = user_path};
    }
}

template <class JsonArray>
void CheckAndFillBlocks(JsonArray blocks_array, std::vector<Graph::Block> &blocks) {
    blocks.resize(blocks_array.Size());
    for (size_t block_id = 0; block_id < blocks_array.Size(); ++block_id) {
        auto &block = blocks[block_id];
        std::unordered_set<std::string> input_names, output_names;
        CheckAndFillIOs(blocks_array[block_id]["inputs"].GetArray(), block.inputs, input_names);
        CheckAndFillIOs(blocks_array[block_id]["outputs"].GetArray(), block.outputs, output_names);
        CheckAndFillExternals(blocks_array[block_id]["externals"].GetArray(), block.externals,
                              input_names, output_names);
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
