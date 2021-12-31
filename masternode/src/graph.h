#pragma once

#include <stdexcept>
#include <string>
#include <utility>

#include <rapidjson/document.h>

struct GraphSemanticError : public std::exception {
    std::string message;

    explicit GraphSemanticError(std::string message = "") : message(std::move(message)) {
    }
};

struct Graph {
    // TODO: fields

    Graph() = default;
    Graph(const rapidjson::Document &graph_document);
};
