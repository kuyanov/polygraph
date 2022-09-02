#pragma once

#include <string>
#include <vector>

#include "serializable.h"
#include "task.h"

struct RunRequest : public Serializable {
    std::string container;
    std::vector<Task> tasks;

    void LoadFromValue(const rapidjson::Value &json) override;
    rapidjson::Value DumpToValue(rapidjson::Document::AllocatorType &alloc) const override;
};
