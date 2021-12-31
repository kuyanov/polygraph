#pragma once

#include <string>

struct Config {
    std::string host;
    int port;
    unsigned int max_payload_size;
    std::string graph_schema_file;

    Config(const char *filename);
};
