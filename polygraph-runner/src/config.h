#pragma once

#include <cstdlib>
#include <string>

#include "environment.h"

class RunnerConfig {
public:
    std::string host;
    int port;
    int id;
    std::string partition;
    unsigned int reconnect_interval_ms;

    static RunnerConfig &Get() {
        static RunnerConfig config;
        return config;
    }

private:
    RunnerConfig() {
        host = GetEnvOrFail("POLYGRAPH_HOST");
        port = atoi(GetEnvOrFail("POLYGRAPH_PORT").c_str());
        id = atoi(GetEnvOrFail("POLYGRAPH_RUNNER_ID").c_str());
        partition = GetEnvOrFail("POLYGRAPH_RUNNER_PARTITION");
        reconnect_interval_ms =
            atoi(GetEnvOrFail("POLYGRAPH_RUNNER_RECONNECT_INTERVAL_MS").c_str());
    }
};
