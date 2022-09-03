#pragma once

#include <string>
#include <vector>

#include "task.h"

struct BlockInput {
    std::string name;
};

struct BlockOutput {
    std::string name;
};

struct Block {
    std::string name;
    std::vector<BlockInput> inputs;
    std::vector<BlockOutput> outputs;
    std::vector<Task> tasks;
};

struct Connection {
    size_t start_block_id, start_output_id, end_block_id, end_input_id;
};

struct Meta {
    std::string name, partition;
    int max_runners;
};

struct Graph {
    std::vector<Block> blocks;
    std::vector<Connection> connections;
    Meta meta;
};
