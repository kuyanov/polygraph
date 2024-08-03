#pragma once

#include <cstdlib>
#include <string>

#include "environment.h"

class TestSchedulerConfig {
public:
    std::string host;
    int port;
    unsigned int max_payload_length;

    static TestSchedulerConfig &Get() {
        static TestSchedulerConfig config;
        return config;
    }

private:
    TestSchedulerConfig() {
        host = GetEnvOr("POLYGRAPH_HOST", "127.0.0.1");
        port = atoi(GetEnvOr("POLYGRAPH_PORT", "3000").c_str());
        max_payload_length = atoi(GetEnvOr("POLYGRAPH_SCHEDULER_MPLEN", "1048576").c_str());
    }
};
