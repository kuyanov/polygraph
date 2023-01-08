#pragma once

#include "constants.h"
#include "json.h"

class Config {
public:
    unsigned int runner_reconnect_interval_ms;
    unsigned int scheduler_max_payload_length;
    unsigned short scheduler_idle_timeout;

    static Config &Get() {
        static Config config(paths::kConfFile);
        return config;
    }

private:
    Config(const std::string &config_path) {
        auto document = ReadJSON(config_path);
        runner_reconnect_interval_ms = document["runner-reconnect-interval-ms"].GetUint();
        scheduler_max_payload_length = document["scheduler-max-payload-length"].GetUint();
        scheduler_idle_timeout = document["scheduler-idle-timeout"].GetUint();
    }
};
