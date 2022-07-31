#pragma once

#include <string>

class Config {
public:
    std::string host;
    int port;
    unsigned int max_payload_size;

    static Config &Instance() {
        static Config config(CONFIG_FILE);
        return config;
    }

private:
    Config(const std::string &config_path);
};
