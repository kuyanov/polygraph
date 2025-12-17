#pragma once

#include <filesystem>
#include <string>

#include "json.h"
#include "serialize.h"

namespace fs = std::filesystem;

class Config {
public:
    std::string host;
    int port;
    int runner_reconnect_interval_ms;
    int runner_timer_interval_ms;
    int scheduler_max_payload_length;
    int scheduler_idle_timeout_s;

    static Config &Get() {
        static Config config;
        return config;
    }

private:
    Config() {
        rapidjson::Document document = ReadJSON(fs::path(CONF_DIR) / "config.json");
        Deserialize(host, document["host"]);
        Deserialize(port, document["port"]);
        Deserialize(runner_reconnect_interval_ms, document["runner_reconnect_interval_ms"]);
        Deserialize(runner_timer_interval_ms, document["runner_timer_interval_ms"]);
        Deserialize(scheduler_max_payload_length, document["scheduler_max_payload_length"]);
        Deserialize(scheduler_idle_timeout_s, document["scheduler_idle_timeout_s"]);
    }
};
