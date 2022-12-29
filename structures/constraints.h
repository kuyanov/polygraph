#pragma once

#include <cstdint>
#include <optional>

struct Constraints {
    std::optional<int64_t> time_limit_ms, wall_time_limit_ms, memory_limit_kb, fsize_limit_kb;
    std::optional<int> max_files, max_threads;
};
