#pragma once

#include <cstdlib>

#include "environment.h"

class TestRunnerConfig {
public:
    int port;
    unsigned int reconnect_interval_ms;

    static TestRunnerConfig &Get() {
        static TestRunnerConfig config;
        return config;
    }

private:
    TestRunnerConfig() {
        port = atoi(GetEnvOr("POLYGRAPH_PORT", "3000").c_str());
        reconnect_interval_ms =
            atoi(GetEnvOr("POLYGRAPH_RUNNER_RECONNECT_INTERVAL_MS", "1000").c_str());
    }
};
