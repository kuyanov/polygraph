#include <fstream>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>

#include "error.h"
#include "schema_validator.h"

SchemaValidator::SchemaValidator(const char *filename) {
    std::ifstream schema_ifs(filename);
    rapidjson::IStreamWrapper schema_isw(schema_ifs);
    rapidjson::Document document;
    document.ParseStream(schema_isw);
    if (document.HasParseError()) {
        throw std::runtime_error("Could not parse json schema: " + FormattedError(document));
    }
    schema_document_.emplace(document);
    schema_validator_.emplace(*schema_document_);
}

rapidjson::Document SchemaValidator::ParseAndValidate(const std::string &json) {
    rapidjson::Document document;
    if (document.Parse(json.c_str()).HasParseError()) {
        throw ParseError(FormattedError(document));
    }
    schema_validator_->Reset();
    if (!document.Accept(*schema_validator_)) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        schema_validator_->GetError().Accept(writer);
        throw ValidationError(buffer.GetString());
    }
    return document;
}
