#pragma once

#include <cstdint>

struct Result {
    int64_t time_usage_ms, time_usage_sys_ms, time_usage_user_ms, wall_time_usage_ms,
        memory_usage_kb;
    bool time_limit_exceeded, wall_time_limit_exceeded, memory_limit_exceeded, exited, signaled,
        oom_killed;
    int exit_code, term_signal;
};
