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

    void Load();
    void Dump();

private:
    fs::path config_path_;

    Config() {
        config_path_ = fs::path(CONF_DIR) / "config.json";
        Load();
    }
};

template <>
inline rapidjson::Value Serialize<Config>(const Config &data,
                                          rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("host", Serialize(data.host, alloc), alloc);
    value.AddMember("port", Serialize(data.port, alloc), alloc);
    value.AddMember("runner_reconnect_interval_ms",
                    Serialize(data.runner_reconnect_interval_ms, alloc), alloc);
    value.AddMember("runner_timer_interval_ms", Serialize(data.runner_timer_interval_ms, alloc),
                    alloc);
    value.AddMember("scheduler_max_payload_length",
                    Serialize(data.scheduler_max_payload_length, alloc), alloc);
    value.AddMember("scheduler_idle_timeout_s", Serialize(data.scheduler_idle_timeout_s, alloc),
                    alloc);
    return value;
}

template <>
inline void Deserialize<Config>(Config &data, const rapidjson::Value &value) {
    Deserialize(data.host, value["host"]);
    Deserialize(data.port, value["port"]);
    Deserialize(data.runner_reconnect_interval_ms, value["runner_reconnect_interval_ms"]);
    Deserialize(data.runner_timer_interval_ms, value["runner_timer_interval_ms"]);
    Deserialize(data.scheduler_max_payload_length, value["scheduler_max_payload_length"]);
    Deserialize(data.scheduler_idle_timeout_s, value["scheduler_idle_timeout_s"]);
}

inline void Config::Load() {
    rapidjson::Document document = ReadJSON(this->config_path_);
    Deserialize(*this, document);
}

inline void Config::Dump() {
    rapidjson::Document document = Serialize(*this);
    WriteJSON(document, config_path_);
}
