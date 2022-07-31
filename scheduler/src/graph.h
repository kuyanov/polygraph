#pragma once

#include <string>
#include <vector>

#include "rapidjson/document.h"

struct Graph {
    struct BlockInput {
        std::string name;
        bool allow_exec;
    };

    struct BlockOutput {
        std::string name;
    };

    struct BlockExternal {
        std::string name;
        std::string user_path;
        bool allow_write;
        bool allow_exec;
    };

    struct Block {
        std::string name;
        std::vector<BlockInput> inputs;
        std::vector<BlockOutput> outputs;
        std::vector<BlockExternal> externals;
        rapidjson::Document tasks{rapidjson::kArrayType};
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
        std::string name;
        std::string partition;
        int max_runners;
    };

    std::vector<Block> blocks;
    std::vector<std::vector<Connection>> connections;
    Meta meta;

    Graph() = default;
    Graph(const rapidjson::Document &graph_document);
};
