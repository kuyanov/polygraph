#pragma once

#include <optional>
#include <string>
#include <rapidjson/document.h>
#include <rapidjson/schema.h>

rapidjson::Document ParseJSON(const std::string &json);
rapidjson::Document ReadJSON(const std::string &path);
std::string StringifyJSON(const rapidjson::Document &document);

class SchemaValidator {
public:
    explicit SchemaValidator(const std::string &filename);

    rapidjson::Document ParseAndValidate(const std::string &json);

private:
    std::optional<rapidjson::SchemaDocument> schema_document_;
    std::optional<rapidjson::SchemaValidator> schema_validator_;
};
