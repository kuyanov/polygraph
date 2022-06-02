#pragma once

#include <optional>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/schema.h>

class SchemaValidator {
public:
    explicit SchemaValidator(const std::string &schema_file);

    rapidjson::Document ParseAndValidate(const std::string &json);

private:
    std::optional<rapidjson::SchemaDocument> schema_document_;
    std::optional<rapidjson::SchemaValidator> schema_validator_;
};
