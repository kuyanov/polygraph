#pragma once

#include <string>
#include <vector>

#include "serializable.h"
#include "task.h"

struct BlockInput : public Serializable {
    std::string name;

    void LoadFromValue(const rapidjson::Value &json) override;
    rapidjson::Value DumpToValue(rapidjson::Document::AllocatorType &alloc) const override;
};

struct BlockOutput : public Serializable {
    std::string name;

    void LoadFromValue(const rapidjson::Value &json) override;
    rapidjson::Value DumpToValue(rapidjson::Document::AllocatorType &alloc) const override;
};

struct Block : public Serializable {
    std::string name;
    std::vector<BlockInput> inputs;
    std::vector<BlockOutput> outputs;
    std::vector<Task> tasks;

    void LoadFromValue(const rapidjson::Value &json) override;
    rapidjson::Value DumpToValue(rapidjson::Document::AllocatorType &alloc) const override;
};

struct Connection : public Serializable {
    size_t start_block_id, start_output_id, end_block_id, end_input_id;

    void LoadFromValue(const rapidjson::Value &json) override;
    rapidjson::Value DumpToValue(rapidjson::Document::AllocatorType &alloc) const override;
};

struct Meta : public Serializable {
    std::string name, partition;
    int max_runners;

    void LoadFromValue(const rapidjson::Value &json) override;
    rapidjson::Value DumpToValue(rapidjson::Document::AllocatorType &alloc) const override;
};

struct Graph : public Serializable {
    std::vector<Block> blocks;
    std::vector<Connection> connections;
    Meta meta;

    void LoadFromValue(const rapidjson::Value &json) override;
    rapidjson::Value DumpToValue(rapidjson::Document::AllocatorType &alloc) const override;
};
