#pragma once

#include <string>

#include "json.h"

class Config {
public:
    std::string scheduler_host, partition;
    int scheduler_port;

    static Config &Instance() {
        static Config config(CONFIG_FILE);
        return config;
    }

private:
    Config(const std::string &config_path) {
        auto config_document = ReadJSON(config_path);
        scheduler_host = config_document["scheduler-host"].GetString();
        scheduler_port = config_document["scheduler-port"].GetInt();
        partition = config_document["partition"].GetString();
    }
};
