#pragma once

#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include "rapidjson/schema.h"
#include "rapidjson/writer.h"

class SchemaValidator {
public:
    std::string validationError;

    explicit SchemaValidator(const char *filename) {
        std::ifstream fin(filename);
        std::string jsonSchema((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
        rapidjson::Document document;
        document.Parse(jsonSchema.c_str());
        if (document.HasParseError()) {
            throw std::runtime_error(
                    "Could not parse json schema: " +
                    std::string(rapidjson::GetParseError_En(document.GetParseError())) +
                    " (at " + std::to_string(document.GetErrorOffset()) + ")");
        }
        schemaDocument = std::make_unique<rapidjson::SchemaDocument>(document);
        schemaValidator = std::make_unique<rapidjson::SchemaValidator>(*schemaDocument);
    }

    bool validate(const rapidjson::Document &document) {
        schemaValidator->Reset();
        validationError = "";
        if (!document.Accept(*schemaValidator)) {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            schemaValidator->GetError().Accept(writer);
            validationError = buffer.GetString();
            return false;
        }
        return true;
    }

private:
    std::unique_ptr<rapidjson::SchemaDocument> schemaDocument;
    std::unique_ptr<rapidjson::SchemaValidator> schemaValidator;
};
