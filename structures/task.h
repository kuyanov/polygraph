#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "serializable.h"

struct Bind : public Serializable {
    std::string inside, outside;
    bool allow_write, allow_exec;

    void LoadFromValue(const rapidjson::Value &json) override;
    rapidjson::Value DumpToValue(rapidjson::Document::AllocatorType &alloc) const override;
};

struct Task : public Serializable {
    std::vector<std::string> argv, env;
    std::vector<Bind> binds;
    std::optional<std::string> _stdin, _stdout, _stderr;
    std::optional<int64_t> time_limit_ms, wall_time_limit_ms, memory_limit_kb, fsize_limit_kb;
    std::optional<int> max_files, max_threads;

    void LoadFromValue(const rapidjson::Value &json) override;
    rapidjson::Value DumpToValue(rapidjson::Document::AllocatorType &alloc) const override;
};
