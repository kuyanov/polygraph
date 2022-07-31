#include "config.h"
#include "json_helpers.h"

Config::Config(const std::string &config_path) {
    auto config_document = ReadJSON(config_path);
    host = config_document["host"].GetString();
    port = config_document["port"].GetInt();
    max_payload_size = config_document["max-payload-size"].GetUint();
}
