#pragma once

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <rapidjson/document.h>

struct GraphSemanticError : public std::exception {
    std::string message;

    explicit GraphSemanticError(std::string message = "") : message(std::move(message)) {
    }
};

struct Graph {
    struct BlockInput {
        std::string name;
        std::optional<std::string> bind_path;

        BlockInput(std::string name, std::optional<std::string> bind_path)
            : name(std::move(name)), bind_path(std::move(bind_path)) {
        }
    };

    struct BlockOutput {
        std::string name;
        std::optional<std::string> bind_path;

        BlockOutput(std::string name, std::optional<std::string> bind_path)
            : name(std::move(name)), bind_path(std::move(bind_path)) {
        }
    };

    struct Block {
        std::string name;
        std::vector<BlockInput> inputs;
        std::unordered_map<std::string, BlockInput *> inputs_by_name;
        std::vector<BlockOutput> outputs;
        std::unordered_map<std::string, BlockOutput *> outputs_by_name;
        std::shared_ptr<rapidjson::Document> tasks;
    };

    struct Connection {
        int start_block_id;
        std::string start_block_output;
        int end_block_id;
        std::string end_block_input;

        Connection(int start_block_id, std::string start_block_output, int end_block_id,
                   std::string end_block_input)
            : start_block_id(start_block_id),
              start_block_output(std::move(start_block_output)),
              end_block_id(end_block_id),
              end_block_input(std::move(end_block_input)) {
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
