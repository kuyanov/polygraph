#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "config.h"

Config ConfigFromFile(const char *filename) {
    std::ifstream fin(filename);
    rapidjson::IStreamWrapper isw(fin);
    rapidjson::Document config;
    config.ParseStream(isw);

    return Config{
            .host = config["host"].GetString(),
            .port = config["port"].GetInt(),
            .max_payload_size = config["max-payload-size"].GetUint(),
            .graph_schema_file = config["graph-schema-file"].GetString(),
    };
}