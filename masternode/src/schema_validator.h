#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include <rapidjson/document.h>
#include <rapidjson/schema.h>

struct GraphParseError : public std::exception {
    std::string message;

    explicit GraphParseError(std::string message = "") : message(std::move(message)) {
    }
};

struct GraphValidationError : public std::exception {
    std::string message;

    explicit GraphValidationError(std::string message = "") : message(std::move(message)) {
    }
};

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
