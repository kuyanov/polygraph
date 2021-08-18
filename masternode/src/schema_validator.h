#pragma once
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include "rapidjson/schema.h"

class SchemaValidator {
public:
    explicit SchemaValidator(const char *filename) {
        std::ifstream fin(filename);
        std::string jsonSchema((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
        rapidjson::Document document;
        document.Parse(jsonSchema.c_str());
        if (document.HasParseError()) {
            // TODO: Detailed information about parsing error
            throw std::runtime_error("Could not parse json schema");
        }
        schemaDocument = std::make_unique<rapidjson::SchemaDocument>(document);
        validator = std::make_unique<rapidjson::SchemaValidator>(*schemaDocument);
        if (!validator->IsValid()) {
            // TODO: Detailed information about schema invalidity
            throw std::runtime_error("Json schema is invalid");
        }
    }

    bool validate(const rapidjson::Document &document) {
        // TODO: Validation details
        return document.Accept(*validator);
    }

private:
    std::unique_ptr<rapidjson::SchemaDocument> schemaDocument;
    std::unique_ptr<rapidjson::SchemaValidator> validator;
};
