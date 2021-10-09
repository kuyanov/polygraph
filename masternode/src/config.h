#pragma once

#include <fstream>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

struct Config {
    std::string host;
    int port;
    std::string ssl_key_file_name, ssl_cert_file_name;
    unsigned int max_payload_size;

    explicit Config(const char* filename) {
        std::ifstream fin(filename);
        rapidjson::IStreamWrapper isw(fin);
        rapidjson::Document config;
        config.ParseStream(isw);

        host = config["host"].GetString();
        port = config["port"].GetInt();
        ssl_key_file_name = config["ssl-key-file-name"].GetString();
        ssl_cert_file_name = config["ssl-cert-file-name"].GetString();
        max_payload_size = config["max-payload-size"].GetInt();
    }
};

Config& GetConfig() {
    static Config config("config.json");
    return config;
}
