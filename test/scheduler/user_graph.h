#pragma once

#include <string>
#include <vector>
#include <rapidjson/document.h>

#include "json_helpers.h"

struct UserGraph {
    struct BlockInput {
        std::string name;
        bool allow_exec = false;
    };

    struct BlockOutput {
        std::string name;
    };

    struct BlockExternal {
        std::string name;
        std::string user_path;
        bool allow_write = false;
        bool allow_exec = false;
    };

    struct Task {
        std::string command;
    };

    struct Block {
        std::string name;
        std::vector<BlockInput> inputs;
        std::vector<BlockOutput> outputs;
        std::vector<BlockExternal> externals;
        std::vector<Task> tasks = {{"test"}};
    };

    struct Connection {
        int start_block_id, start_output_id;
        int end_block_id, end_input_id;
    };

    struct Meta {
        std::string name = "sample graph";
        std::string partition = "all";
        int max_runners = INT_MAX;
    };

    std::vector<Block> blocks;
    std::vector<Connection> connections;
    Meta meta;
};

template <class Allocator>
rapidjson::Value BuildBlocks(const std::vector<UserGraph::Block> &graph_blocks, Allocator &alloc) {
    rapidjson::Value blocks(rapidjson::kArrayType);
    for (const auto &graph_block : graph_blocks) {
        rapidjson::Value block(rapidjson::kObjectType);
        block.AddMember("name", rapidjson::Value().SetString(graph_block.name.c_str(), alloc),
                        alloc);
        rapidjson::Value inputs(rapidjson::kArrayType);
        for (const auto &graph_input : graph_block.inputs) {
            rapidjson::Value input(rapidjson::kObjectType);
            input.AddMember("name", rapidjson::Value().SetString(graph_input.name.c_str(), alloc),
                            alloc);
            input.AddMember("allow-exec", rapidjson::Value().SetBool(graph_input.allow_exec),
                            alloc);
            inputs.PushBack(input, alloc);
        }
        block.AddMember("inputs", inputs, alloc);
        rapidjson::Value outputs(rapidjson::kArrayType);
        for (const auto &graph_output : graph_block.outputs) {
            rapidjson::Value output(rapidjson::kObjectType);
            output.AddMember("name", rapidjson::Value().SetString(graph_output.name.c_str(), alloc),
                             alloc);
            outputs.PushBack(output, alloc);
        }
        block.AddMember("outputs", outputs, alloc);
        rapidjson::Value externals(rapidjson::kArrayType);
        for (const auto &graph_external : graph_block.externals) {
            rapidjson::Value external(rapidjson::kObjectType);
            external.AddMember(
                "name", rapidjson::Value().SetString(graph_external.name.c_str(), alloc), alloc);
            external.AddMember(
                "user-path", rapidjson::Value().SetString(graph_external.user_path.c_str(), alloc),
                alloc);
            external.AddMember("allow-write",
                               rapidjson::Value().SetBool(graph_external.allow_write), alloc);
            external.AddMember("allow-exec", rapidjson::Value().SetBool(graph_external.allow_exec),
                               alloc);
            externals.PushBack(external, alloc);
        }
        block.AddMember("externals", externals, alloc);
        rapidjson::Value tasks(rapidjson::kArrayType);
        for (const auto &graph_task : graph_block.tasks) {
            rapidjson::Value task(rapidjson::kObjectType);
            rapidjson::Value argv(rapidjson::kArrayType);
            argv.PushBack(rapidjson::Value().SetString(graph_task.command.c_str(), alloc), alloc);
            task.AddMember("argv", argv, alloc);
            tasks.PushBack(task, alloc);
        }
        block.AddMember("tasks", tasks, alloc);
        blocks.PushBack(block, alloc);
    }
    return blocks;
}

template <class Allocator>
rapidjson::Value BuildConnections(const std::vector<UserGraph::Connection> &graph_connections,
                                  Allocator &alloc) {
    rapidjson::Value connections(rapidjson::kArrayType);
    for (const auto &graph_connection : graph_connections) {
        rapidjson::Value connection(rapidjson::kObjectType);
        connection.AddMember("start-block-id", graph_connection.start_block_id, alloc);
        connection.AddMember("start-output-id", graph_connection.start_output_id, alloc);
        connection.AddMember("end-block-id", graph_connection.end_block_id, alloc);
        connection.AddMember("end-input-id", graph_connection.end_input_id, alloc);
        connections.PushBack(connection, alloc);
    }
    return connections;
}

template <class Allocator>
rapidjson::Value BuildMeta(const UserGraph::Meta &graph_meta, Allocator &alloc) {
    rapidjson::Value meta(rapidjson::kObjectType);
    meta.AddMember("name", rapidjson::Value().SetString(graph_meta.name.c_str(), alloc), alloc);
    meta.AddMember("partition", rapidjson::Value().SetString(graph_meta.partition.c_str(), alloc),
                   alloc);
    meta.AddMember("max-runners", graph_meta.max_runners, alloc);
    return meta;
}

std::string StringifyGraph(const UserGraph &graph) {
    rapidjson::Document body(rapidjson::kObjectType);
    auto &alloc = body.GetAllocator();
    body.AddMember("blocks", BuildBlocks(graph.blocks, alloc), alloc);
    body.AddMember("connections", BuildConnections(graph.connections, alloc), alloc);
    body.AddMember("meta", BuildMeta(graph.meta, alloc), alloc);
    return StringifyJSON(body);
}
