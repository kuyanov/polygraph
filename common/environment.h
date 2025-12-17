#pragma once

#include <cstdlib>
#include <iostream>
#include <string>

inline std::string GetEnvOr(const char *env_name, const std::string &default_value) {
    const char *value = getenv(env_name);
    if (!value) {
        return default_value;
    }
    return value;
}

inline std::string GetEnvOrFail(const char *env_name) {
    const char *value = getenv(env_name);
    if (!value) {
        std::cerr << "Environment variable " << env_name << " was not specified, exiting."
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    return value;
}
