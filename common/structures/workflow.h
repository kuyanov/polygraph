#pragma once

#include <string>
#include <vector>

#include "bind.h"
#include "constraints.h"

struct Input {
    std::string path;
    bool cached;
};

struct Output {
    std::string path;
};

struct Block {
    std::string name;
    std::vector<Input> inputs;
    std::vector<Output> outputs;
    std::vector<Bind> binds;
    std::vector<std::string> argv, env;
    Constraints constraints;
};

struct Connection {
    size_t source_block_id, source_output_id, target_block_id, target_input_id;
};

struct Meta {
    std::string name, partition;
    int max_runners;
};

struct Workflow {
    std::vector<Block> blocks;
    std::vector<Connection> connections;
    Meta meta;
};
