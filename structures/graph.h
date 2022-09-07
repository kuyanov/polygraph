#pragma once

#include <string>
#include <vector>

#include "task.h"

struct Bind {
    std::string inside, outside;
    int permissions;
};

struct Input {
    std::string name;
};

struct Output {
    std::string name;
};

struct Block {
    std::string name;
    std::vector<Bind> binds;
    std::vector<Input> inputs;
    std::vector<Output> outputs;
    Task task;
};

struct Connection {
    int start_block_id, start_output_id, end_block_id, end_input_id;
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
