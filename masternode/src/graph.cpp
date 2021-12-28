#include "graph.h"

bool GraphStorage::Contains(const std::string &graph_id) {
    return graphs_.contains(graph_id);
}

void GraphStorage::InitGraph(const std::string &graph_id, const rapidjson::Document &graph) {
    graphs_[graph_id] = 57;  // TODO: Initialize graph
}
