#pragma once

#include <cstdlib>

#include "environment.h"

class SchedulerConfig {
public:
    int port;
    unsigned int max_payload_length;
    unsigned short idle_timeout;

    static SchedulerConfig &Get() {
        static SchedulerConfig config;
        return config;
    }

private:
    SchedulerConfig() {
        port = atoi(GetEnvOrFail("POLYGRAPH_PORT").c_str());
        max_payload_length = atoi(GetEnvOrFail("POLYGRAPH_SCHEDULER_MPLEN").c_str());
        idle_timeout = atoi(GetEnvOrFail("POLYGRAPH_SCHEDULER_IDLE_TIMEOUT_S").c_str());
    }
};
