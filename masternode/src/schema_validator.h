#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include <rapidjson/schema.h>

struct ParseError : public std::exception {
    std::string message;

    explicit ParseError(std::string message = "") : message(std::move(message)) {
    }
};

struct ValidationError : public std::exception {
    std::string message;

    explicit ValidationError(std::string message = "") : message(std::move(message)) {
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
    rapidjson::Document document_;
    std::optional<rapidjson::SchemaDocument> schema_document_;
    // std::optional<rapidjson::SchemaValidator> schema_validator_;
};
