#pragma once

#include <string>

struct Config {
    std::string host;
    int port;
    unsigned int max_payload_size;

    Config(const std::string &config_path);
};
