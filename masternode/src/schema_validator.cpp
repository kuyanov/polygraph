#include <fstream>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>

#include "schema_validator.h"

SchemaValidator::SchemaValidator(const char *filename) {
    std::ifstream schema_ifs(filename);
    rapidjson::IStreamWrapper schema_isw(schema_ifs);
    rapidjson::Document document;
    document.ParseStream(schema_isw);
    if (document.HasParseError()) {
        throw std::runtime_error("Could not parse json schema: " + FormattedError(document));
    }
    schema_document_.emplace(std::move(document));
    // schema_validator_.emplace(*schema_document_);
}

rapidjson::Document SchemaValidator::ParseAndValidate(const std::string &json) {
    rapidjson::Document document;
    if (document.Parse(json.c_str()).HasParseError()) {
        throw ParseError(FormattedError(document));
    }
    // schema_validator_->Reset();
    rapidjson::SchemaValidator schema_validator(*schema_document_);
    if (!document.Accept(schema_validator)) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        schema_validator.GetError().Accept(writer);
        throw ValidationError(buffer.GetString());
    }
    return document;
}
