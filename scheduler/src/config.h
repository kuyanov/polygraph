#pragma once

#include <string>

#include "json.h"

class Config {
public:
    std::string host;
    int port;
    unsigned int max_payload_length;
    unsigned short idle_timeout;

    static Config &Get() {
        static Config config;
        return config;
    }

    void Load(const std::string &config_path) {
        auto document = ReadJSON(config_path);
        host = document["host"].GetString();
        port = document["port"].GetInt();
        max_payload_length = document["max-payload-length"].GetUint();
        idle_timeout = document["idle-timeout"].GetUint();
    }
};
