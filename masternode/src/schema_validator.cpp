#include <fstream>
#include <iterator>

#include <rapidjson/writer.h>

#include "schema_validator.h"

SchemaValidator::SchemaValidator(const char *filename) {
    std::ifstream fin(filename);
    std::string json_schema((std::istreambuf_iterator<char>(fin)),
                            std::istreambuf_iterator<char>());
    rapidjson::Document document;
    document.Parse(json_schema.c_str());
    if (document.HasParseError()) {
        throw std::runtime_error("Could not parse json schema: " + FormattedError(document));
    }
    schema_document_ = std::make_unique<rapidjson::SchemaDocument>(std::move(document));
    schema_validator_ = std::make_unique<rapidjson::SchemaValidator>(*schema_document_);
}

rapidjson::Document SchemaValidator::Parse(const std::string &graph_json) {
    rapidjson::Document graph;
    if (graph.Parse(graph_json.c_str()).HasParseError()) {
        throw ParseError(FormattedError(graph));
    }
    schema_validator_->Reset();
    if (!graph.Accept(*schema_validator_)) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        schema_validator_->GetError().Accept(writer);
        throw ValidationError(buffer.GetString());
    }
    return graph;
}
