#pragma once

#include <string>

#include "json.h"

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
    Config(const std::string &config_path) {
        auto config_document = ReadJSON(config_path);
        host = "127.0.0.1";
        port = config_document["port"].GetInt();
        max_payload_size = config_document["max-payload-size"].GetUint();
    }
};
