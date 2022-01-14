#pragma once

#include <optional>
#include <string>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

struct UserGraph {
    struct BlockInput {
        std::string name;
        std::optional<std::string> bind_path;
    };

    struct BlockOutput {
        std::string name;
        std::optional<std::string> bind_path;
    };

    struct Task {
        std::string command;
    };

    struct Block {
        std::string name;
        std::vector<BlockInput> inputs;
        std::vector<BlockOutput> outputs;
        std::vector<Task> tasks;
    };

    struct Connection {
        int start_block_id, start_output_id;
        int end_block_id, end_input_id;
    };

    struct Meta {
        std::string runner_group = "all";
        int max_runners = INT_MAX;
    };

    std::vector<Block> blocks;
    std::vector<Connection> connections;
    Meta meta;
};

std::string StringifyGraph(const UserGraph &graph) {
    rapidjson::Document body(rapidjson::kObjectType);
    auto &alloc = body.GetAllocator();
    rapidjson::Value blocks(rapidjson::kArrayType);
    for (const auto &graph_block : graph.blocks) {
        rapidjson::Value block(rapidjson::kObjectType);
        block.AddMember("name", rapidjson::Value().SetString(graph_block.name.c_str(), alloc),
                        alloc);
        rapidjson::Value inputs(rapidjson::kArrayType);
        for (const auto &graph_input : graph_block.inputs) {
            rapidjson::Value input(rapidjson::kObjectType);
            input.AddMember("name", rapidjson::Value().SetString(graph_input.name.c_str(), alloc),
                            alloc);
            if (graph_input.bind_path) {
                input.AddMember("bind-path",
                                rapidjson::Value().SetString(graph_input.bind_path->c_str(), alloc),
                                alloc);
            }
            inputs.PushBack(input, alloc);
        }
        block.AddMember("inputs", inputs, alloc);
        rapidjson::Value outputs(rapidjson::kArrayType);
        for (const auto &graph_output : graph_block.outputs) {
            rapidjson::Value output(rapidjson::kObjectType);
            output.AddMember("name", rapidjson::Value().SetString(graph_output.name.c_str(), alloc),
                             alloc);
            if (graph_output.bind_path) {
                output.AddMember(
                    "bind-path",
                    rapidjson::Value().SetString(graph_output.bind_path->c_str(), alloc), alloc);
            }
            outputs.PushBack(output, alloc);
        }
        block.AddMember("outputs", outputs, alloc);
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
    body.AddMember("blocks", blocks, alloc);
    rapidjson::Value connections(rapidjson::kArrayType);
    for (const auto &graph_connection : graph.connections) {
        rapidjson::Value connection(rapidjson::kObjectType);
        connection.AddMember("start-block-id", graph_connection.start_block_id, alloc);
        connection.AddMember("start-output-id", graph_connection.start_output_id, alloc);
        connection.AddMember("end-block-id", graph_connection.end_block_id, alloc);
        connection.AddMember("end-input-id", graph_connection.end_input_id, alloc);
        connections.PushBack(connection, alloc);
    }
    body.AddMember("connections", connections, alloc);
    rapidjson::Value meta(rapidjson::kObjectType);
    meta.AddMember("runner-group",
                   rapidjson::Value().SetString(graph.meta.runner_group.c_str(), alloc), alloc);
    meta.AddMember("max-runners", graph.meta.max_runners, alloc);
    body.AddMember("meta", meta, alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    body.Accept(writer);
    return buffer.GetString();
}
