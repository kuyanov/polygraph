#include <fstream>
#include <string>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>

#include "error.h"
#include "schema_validator.h"

std::string FormattedError(const rapidjson::Document &document) {
    return std::string(rapidjson::GetParseError_En(document.GetParseError())) + " (at position " +
           std::to_string(document.GetErrorOffset()) + ")";
}

SchemaValidator::SchemaValidator(const std::string &schema_path) {
    std::ifstream schema_ifs(schema_path);
    rapidjson::IStreamWrapper schema_isw(schema_ifs);
    rapidjson::Document schema_document;
    schema_document.ParseStream(schema_isw);
    if (schema_document.HasParseError()) {
        throw std::runtime_error("Could not parse json schema: " + FormattedError(schema_document));
    }
    schema_document_.emplace(schema_document);
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
