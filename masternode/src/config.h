#pragma once

#include <fstream>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

struct Config {
    std::string host;
    int port;
    int maxPayloadSize;

    explicit Config(const char *filename) {
        std::ifstream fin(filename);
        rapidjson::IStreamWrapper isw(fin);
        rapidjson::Document d;
        d.ParseStream(isw);

        host = d["host"].GetString();
        port = d["port"].GetInt();
        maxPayloadSize = d["max-payload-size"].GetInt();
    }
};
