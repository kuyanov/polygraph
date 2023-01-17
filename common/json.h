#pragma once

#include <optional>
#include <string>
#include <rapidjson/document.h>
#include <rapidjson/schema.h>

rapidjson::Document ParseJSON(const std::string &text);
std::string StringifyJSON(const rapidjson::Document &document);
rapidjson::Document ReadJSON(const std::string &path);
void WriteJSON(const rapidjson::Document &document, const std::string &path);

class SchemaValidator {
public:
    explicit SchemaValidator(const std::string &schema_path);

    rapidjson::Document ParseAndValidate(const std::string &text);

private:
    std::optional<rapidjson::SchemaDocument> schema_document_;
    std::optional<rapidjson::SchemaValidator> schema_validator_;
};
