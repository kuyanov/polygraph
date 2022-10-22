#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct Task {
    std::vector<std::string> argv, env;
    std::optional<int64_t> time_limit_ms, wall_time_limit_ms, memory_limit_kb, fsize_limit_kb;
    std::optional<int> max_files, max_threads;
};

struct RunRequest {
    std::string container;
    Task task;
};
