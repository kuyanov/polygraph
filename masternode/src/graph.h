#pragma once

#include <optional>
#include <string>
#include <vector>

#include <rapidjson/document.h>

struct Graph {
    struct BlockInput {
        std::string name;
        std::optional<std::string> bind_path;
    };

    struct BlockOutput {
        std::string name;
        std::optional<std::string> bind_path;
    };

    struct Block {
        std::string name;
        std::vector<BlockInput> inputs;
        std::vector<BlockOutput> outputs;
        rapidjson::Document tasks;
    };

    struct Connection {
        size_t start_block_id, start_output_id;
        size_t end_block_id, end_input_id;

        Connection(size_t start_block_id, size_t start_output_id, size_t end_block_id,
                   size_t end_input_id)
            : start_block_id(start_block_id),
              start_output_id(start_output_id),
              end_block_id(end_block_id),
              end_input_id(end_input_id) {
        }
    };

    struct Meta {
        std::string runner_group;
        int max_runners;
    };

    std::vector<Block> blocks;
    std::vector<std::vector<Connection>> go;
    Meta meta;

    Graph() = default;
    Graph(const rapidjson::Document &graph_document);
};
