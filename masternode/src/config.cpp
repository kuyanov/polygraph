#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "config.h"

Config::Config(const std::string &config_path) {
    std::ifstream config_ifs(config_path);
    rapidjson::IStreamWrapper config_isw(config_ifs);
    rapidjson::Document config_document;
    config_document.ParseStream(config_isw);

    host = config_document["host"].GetString();
    port = config_document["port"].GetInt();
    max_payload_size = config_document["max-payload-size"].GetUint();
}
