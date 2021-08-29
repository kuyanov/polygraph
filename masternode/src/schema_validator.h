#pragma once

#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include "rapidjson/schema.h"
#include "rapidjson/writer.h"

struct ParseError : public std::exception {
    std::string message;

    explicit ParseError(std::string message_ = "") : message(std::move(message_)) {}
};

struct ValidationError : public std::exception {
    std::string message;

    explicit ValidationError(std::string message_ = "") : message(std::move(message_)) {}
};

std::string formattedError(const rapidjson::Document &document) {
    return std::string(rapidjson::GetParseError_En(document.GetParseError())) +
           " (at position " + std::to_string(document.GetErrorOffset()) + ")";
}

class SchemaValidator {
public:
    explicit SchemaValidator(const char *filename) {
        std::ifstream fin(filename);
        std::string jsonSchema((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
        rapidjson::Document document;
        document.Parse(jsonSchema.c_str());
        if (document.HasParseError()) {
            throw std::runtime_error("Could not parse json schema: " + formattedError(document));
        }
        schemaDocument = std::make_unique<rapidjson::SchemaDocument>(document);
        schemaValidator = std::make_unique<rapidjson::SchemaValidator>(*schemaDocument);
    }

    rapidjson::Document parse(const std::string &graphJson) {
        rapidjson::Document graph;
        if (graph.Parse(graphJson.c_str()).HasParseError()) {
            throw ParseError(formattedError(graph));
        }
        schemaValidator->Reset();
        if (!graph.Accept(*schemaValidator)) {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            schemaValidator->GetError().Accept(writer);
            throw ValidationError(buffer.GetString());
        }
        return graph;
    }

private:
    std::unique_ptr<rapidjson::SchemaDocument> schemaDocument;
    std::unique_ptr<rapidjson::SchemaValidator> schemaValidator;
};
