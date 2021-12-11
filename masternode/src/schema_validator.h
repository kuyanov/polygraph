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
    explicit SchemaValidator(const char *filename) {
        std::ifstream fin(filename);
        std::string json_schema((std::istreambuf_iterator<char>(fin)),
                                std::istreambuf_iterator<char>());
        rapidjson::Document document;
        document.Parse(json_schema.c_str());
        if (document.HasParseError()) {
            throw std::runtime_error("Could not parse json schema: " + FormattedError(document));
        }
        schema_document_ = std::make_unique<rapidjson::SchemaDocument>(document);
        schema_validator_ = std::make_unique<rapidjson::SchemaValidator>(*schema_document_);
    }

    rapidjson::Document Parse(const std::string &graph_json) {
        rapidjson::Document graph;
        if (graph.Parse(graph_json.c_str()).HasParseError()) {
            throw ParseError(FormattedError(graph));
        }
        schema_validator_->Reset();
        if (!graph.Accept(*schema_validator_)) {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            schema_validator_->GetError().Accept(writer);
            throw ValidationError(buffer.GetString());
        }
        return graph;
    }

private:
    std::unique_ptr<rapidjson::SchemaDocument> schema_document_;
    std::unique_ptr<rapidjson::SchemaValidator> schema_validator_;
};
