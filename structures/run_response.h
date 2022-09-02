#pragma once

#include <string>
#include <vector>

#include "serializable.h"
#include "status.h"

struct RunResponse : public Serializable {
    bool has_error;
    std::string error;
    std::vector<Status> statuses;

    void LoadFromValue(const rapidjson::Value &json) override;
    rapidjson::Value DumpToValue(rapidjson::Document::AllocatorType &alloc) const override;
};

struct BlockRunResponse : public RunResponse {
    size_t block_id;

    BlockRunResponse(const RunResponse &other);

    void LoadFromValue(const rapidjson::Value &json) override;
    rapidjson::Value DumpToValue(rapidjson::Document::AllocatorType &alloc) const override;
};
