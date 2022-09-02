#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>

#include "error.h"
#include "json.h"

namespace fs = std::filesystem;

rapidjson::Document ParseJSON(const std::string &s) {
    rapidjson::Document document;
    document.Parse(s.c_str());
    return document;
}

rapidjson::Document ReadJSON(const std::string &path) {
    std::ifstream ifs(path);
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document document;
    document.ParseStream(isw);
    return document;
}

std::string StringifyJSON(const rapidjson::Document &document) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    return buffer.GetString();
}

std::string FormattedError(const rapidjson::Document &document) {
    return std::string(rapidjson::GetParseError_En(document.GetParseError())) + " (at position " +
           std::to_string(document.GetErrorOffset()) + ")";
}

SchemaValidator::SchemaValidator(const std::string &filename) {
    auto document = ReadJSON((fs::path(SCHEMA_DIR) / filename).string());
    if (document.HasParseError()) {
        throw std::runtime_error("Could not parse json schema: " + FormattedError(document));
    }
    schema_document_.emplace(document);
    schema_validator_.emplace(*schema_document_);
}

rapidjson::Document SchemaValidator::ParseAndValidate(const std::string &s) {
    auto document = ParseJSON(s);
    if (document.HasParseError()) {
        throw ParseError(FormattedError(document));
    }
    schema_validator_->Reset();
    if (!document.Accept(*schema_validator_)) {
        rapidjson::Document error;
        error.CopyFrom(schema_validator_->GetError(), error.GetAllocator());
        throw ValidationError(StringifyJSON(error));
    }
    return document;
}
