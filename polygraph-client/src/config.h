#pragma once

#include <cstdlib>
#include <string>

#include "environment.h"

class ClientConfig {
public:
    std::string host;
    int port;

    static ClientConfig &Get() {
        static ClientConfig config;
        return config;
    }

private:
    ClientConfig() {
        host = GetEnvOrFail("POLYGRAPH_HOST");
        port = atoi(GetEnvOrFail("POLYGRAPH_PORT").c_str());
    }
};
