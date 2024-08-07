#pragma once

#include <cstdint>

struct RunStatus {
    bool exited, signaled, time_limit_exceeded, wall_time_limit_exceeded, memory_limit_exceeded,
        oom_killed;
    int exit_code, term_signal;
    int64_t time_usage_ms, time_usage_sys_ms, time_usage_user_ms, wall_time_usage_ms,
        memory_usage_kb;
};
