#pragma once

#include <limits.h>
#include <string>
#include <vector>

#include "graph.h"

struct TestBlock {
    std::vector<std::string> inputs, outputs;
};

struct TestConnection {
    int start_block_id, start_output_id, end_block_id, end_input_id;
};

struct TestMeta {
    std::string partition = "all";
    int max_runners = INT_MAX;
};

struct TestGraph {
    std::vector<TestBlock> blocks;
    std::vector<TestConnection> connections;
    TestMeta meta;
};

Graph BuildGraph(const TestGraph &test_graph) {
    Graph graph;
    for (const auto &block : test_graph.blocks) {
        graph.blocks.emplace_back();
        for (const auto &input_name : block.inputs) {
            BlockInput input;
            input.name = input_name;
            graph.blocks.back().inputs.push_back(input);
        }
        for (const auto &output_name : block.outputs) {
            BlockOutput output;
            output.name = output_name;
            graph.blocks.back().outputs.push_back(output);
        }
        graph.blocks.back().tasks.emplace_back();
    }
    for (const auto &test_connection : test_graph.connections) {
        Connection connection;
        connection.start_block_id = test_connection.start_block_id;
        connection.start_output_id = test_connection.start_output_id;
        connection.end_block_id = test_connection.end_block_id;
        connection.end_input_id = test_connection.end_input_id;
        graph.connections.push_back(connection);
    }
    graph.meta.partition = test_graph.meta.partition;
    graph.meta.max_runners = test_graph.meta.max_runners;
    return graph;
}
