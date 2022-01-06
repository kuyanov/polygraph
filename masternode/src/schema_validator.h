#pragma once

#include <optional>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/schema.h>

inline std::string FormattedError(const rapidjson::Document &document) {
    return std::string(rapidjson::GetParseError_En(document.GetParseError())) + " (at position " +
           std::to_string(document.GetErrorOffset()) + ")";
}

class SchemaValidator {
public:
    explicit SchemaValidator(const char *filename);

    rapidjson::Document ParseAndValidate(const std::string &graph_json);

private:
    std::optional<rapidjson::SchemaDocument> schema_document_;
    std::optional<rapidjson::SchemaValidator> schema_validator_;
};
