#pragma once

#include <string>
#include <unordered_map>

#include <rapidjson/document.h>

class GraphStorage {
public:
    bool Contains(const std::string &graph_id);

    void InitGraph(const std::string &graph_id, const rapidjson::Document &graph);

private:
    std::unordered_map<std::string, int> graphs_;
};
