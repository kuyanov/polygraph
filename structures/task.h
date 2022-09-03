#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct Bind {
    std::string inside, outside;
    bool allow_write, allow_exec;
};

struct Task {
    std::vector<std::string> argv, env;
    std::vector<Bind> binds;
    std::optional<std::string> stdin_, stdout_, stderr_;
    std::optional<int64_t> time_limit_ms, wall_time_limit_ms, memory_limit_kb, fsize_limit_kb;
    std::optional<int> max_files, max_threads;
};
