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
        auto config_document = ReadJSON(config_path);
        scheduler_host = config_document["scheduler-host"].GetString();
        scheduler_port = config_document["scheduler-port"].GetInt();
        partition = config_document["partition"].GetString();
        reconnect_interval_ms = config_document["reconnect-interval-ms"].GetInt();
    }
};
