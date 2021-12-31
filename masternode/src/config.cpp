#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "config.h"

Config::Config(const char *filename) {
    std::ifstream config_ifs(filename);
    rapidjson::IStreamWrapper config_isw(config_ifs);
    rapidjson::Document config;
    config.ParseStream(config_isw);

    host = config["host"].GetString();
    port = config["port"].GetInt();
    max_payload_size = config["max-payload-size"].GetUint();
    graph_schema_file = config["graph-schema-file"].GetString();
}
