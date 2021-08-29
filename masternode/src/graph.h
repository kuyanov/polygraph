#pragma once

#include <string>
#include <unordered_map>
#include "rapidjson/document.h"

class GraphsStorage {
public:
    bool contains(const std::string &graphId) {
        return graphs.contains(graphId);
    }

    void initGraph(const std::string &graphId, const rapidjson::Document &graph) {
        graphs[graphId] = 57; // TODO: Initialize graph
    }

private:
    std::unordered_map<std::string, int> graphs;
};
