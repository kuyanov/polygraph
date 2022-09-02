#pragma once

#include <string>

#include "json.h"

class Config {
public:
    std::string scheduler_host, partition;
    int scheduler_port, reconnect_interval_ms;

    static Config &Get() {
        static Config config(CONFIG_FILE);
        return config;
    }

private:
    Config(const std::string &config_path) {
        auto document = ReadJSON(config_path);
        scheduler_host = document["scheduler-host"].GetString();
        scheduler_port = document["scheduler-port"].GetInt();
        partition = document["partition"].GetString();
        reconnect_interval_ms = document["reconnect-interval-ms"].GetInt();
    }
};
