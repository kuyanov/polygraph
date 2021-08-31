#pragma once

#include <fstream>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

struct Config {
    std::string host;
    int port;
    std::string sslKeyFileName, sslCertFileName;
    unsigned int maxPayloadSize;

    explicit Config(const char *filename) {
        std::ifstream fin(filename);
        rapidjson::IStreamWrapper isw(fin);
        rapidjson::Document config;
        config.ParseStream(isw);

        host = config["host"].GetString();
        port = config["port"].GetInt();
        sslKeyFileName = config["ssl-key-file-name"].GetString();
        sslCertFileName = config["ssl-cert-file-name"].GetString();
        maxPayloadSize = config["max-payload-size"].GetInt();
    }
};
