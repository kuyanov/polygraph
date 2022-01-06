#pragma once

#include <stdexcept>
#include <string>
#include <utility>

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

struct SemanticError : public std::exception {
    std::string message;

    explicit SemanticError(std::string message = "") : message(std::move(message)) {
    }
};
