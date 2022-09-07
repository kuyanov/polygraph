#pragma once

#include <string>

#include "json.h"

class Config {
public:
    std::string scheduler_host;
    int scheduler_port;

    static Config &Get() {
        static Config config(CONFIG_FILE);
        return config;
    }

private:
    Config(const std::string &config_path) {
        auto document = ReadJSON(config_path);
        scheduler_host = document["scheduler-host"].GetString();
        scheduler_port = document["scheduler-port"].GetInt();
    }
};
