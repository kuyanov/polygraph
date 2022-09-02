#pragma once

#include <string>

#include "json.h"

class Config {
public:
    std::string host;
    int port;
    unsigned int max_payload_size;

    static Config &Get() {
        static Config config(CONFIG_FILE);
        return config;
    }

private:
    Config(const std::string &config_path) {
        auto document = ReadJSON(config_path);
        host = document["host"].GetString();
        port = document["port"].GetInt();
        max_payload_size = document["max-payload-size"].GetUint();
    }
};
