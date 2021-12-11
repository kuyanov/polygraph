#pragma once

#include <string>
#include <unordered_map>

#include "rapidjson/document.h"

class GraphStorage {
public:
    bool Contains(const std::string &graph_id) {
        return graphs_.contains(graph_id);
    }

    void InitGraph(const std::string &graph_id, const rapidjson::Document &graph) {
        graphs_[graph_id] = 57;  // TODO: Initialize graph
    }

private:
    std::unordered_map<std::string, int> graphs_;
};
