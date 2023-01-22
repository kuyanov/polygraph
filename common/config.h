#pragma once

#include <string>

#include "environment.h"
#include "json.h"

class Config {
public:
    unsigned int runner_reconnect_interval_ms;
    std::string scheduler_host;
    int scheduler_port;
    unsigned int scheduler_max_payload_length;
    unsigned short scheduler_idle_timeout;
    std::string user_dir;

    static Config &Get() {
        static Config config;
        return config;
    }

    void Load() {
        auto document = ReadJSON(GetConfDir() + "/config.json");
        runner_reconnect_interval_ms = document["runner-reconnect-interval-ms"].GetUint();
        scheduler_host = document["scheduler-host"].GetString();
        scheduler_port = document["scheduler-port"].GetInt();
        scheduler_max_payload_length = document["scheduler-max-payload-length"].GetUint();
        scheduler_idle_timeout = document["scheduler-idle-timeout"].GetUint();
        user_dir = document["user-dir"].GetString();
    }

    void Dump() {
        rapidjson::Document document(rapidjson::kObjectType);
        auto &alloc = document.GetAllocator();
        document.AddMember("runner-reconnect-interval-ms",
                           rapidjson::Value().SetUint(runner_reconnect_interval_ms), alloc);
        document.AddMember("scheduler-host",
                           rapidjson::Value().SetString(scheduler_host.c_str(), alloc), alloc);
        document.AddMember("scheduler-port", rapidjson::Value().SetInt(scheduler_port), alloc);
        document.AddMember("scheduler-max-payload-length",
                           rapidjson::Value().SetUint(scheduler_max_payload_length), alloc);
        document.AddMember("scheduler-idle-timeout",
                           rapidjson::Value().SetUint(scheduler_idle_timeout), alloc);
        document.AddMember("user-dir", rapidjson::Value().SetString(user_dir.c_str(), alloc),
                           alloc);
        WriteJSON(document, GetConfDir() + "/config.json");
    }

private:
    Config() {
        Load();
    }
};
