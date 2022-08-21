#pragma once

#include <exception>
#include <string>

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

struct APIError : public std::exception {
    std::string message;

    explicit APIError(std::string message = "") : message(std::move(message)) {
    }
};
