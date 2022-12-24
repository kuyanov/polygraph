#pragma once

#include <string>
#include <vector>

#include "task.h"

struct Bind {
    std::string inside_filename, outside_path;
};

struct Block {
    std::string name;
    std::vector<Bind> binds;
    Task task;
};

struct Connection {
    std::string type;
    size_t start_block_id, end_block_id;
    std::string start_filename, end_filename;
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
