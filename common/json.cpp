#include <fstream>
#include <stdexcept>
#include <string>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include "error.h"
#include "json.h"

rapidjson::Document ParseJSON(const std::string &text) {
    rapidjson::Document document;
    document.Parse(text.c_str());
    return document;
}

std::string StringifyJSON(const rapidjson::Document &document) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer writer(buffer);
    document.Accept(writer);
    return buffer.GetString();
}

rapidjson::Document ReadJSON(const std::string &path) {
    std::ifstream ifs(path);
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document document;
    document.ParseStream(isw);
    return document;
}

void WriteJSON(const rapidjson::Document &document, const std::string &path) {
    std::ofstream ofs(path);
    rapidjson::OStreamWrapper osw(ofs);
    rapidjson::PrettyWriter writer(osw);
    writer.SetIndent(' ', 2);
    document.Accept(writer);
}

std::string FormattedError(const rapidjson::Document &document) {
    return std::string(rapidjson::GetParseError_En(document.GetParseError())) + " (at position " +
           std::to_string(document.GetErrorOffset()) + ")";
}

SchemaValidator::SchemaValidator(const std::string &schema_path) {
    auto document = ReadJSON(schema_path);
    if (document.HasParseError()) {
        throw std::runtime_error("Could not parse json schema: " + FormattedError(document));
    }
    schema_document_.emplace(document);
    schema_validator_.emplace(*schema_document_);
}

rapidjson::Document SchemaValidator::ParseAndValidate(const std::string &text) {
    auto document = ParseJSON(text);
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
