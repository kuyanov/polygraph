#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <boost/beast.hpp>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include "config.h"
#include "run.h"

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;

class MasterNode {
public:
    Config config;

    MasterNode(const Config &config = Config("../masternode/config/test.json")) : config(config) {
        std::thread([*this] { Run(this->config); }).detach();
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
};

class HttpSession {
public:
    HttpSession(std::shared_ptr<MasterNode> server) : server_(std::move(server)), stream_(ioc_) {
        ip::tcp::resolver resolver(ioc_);
        auto results = resolver.resolve(server_->config.host, std::to_string(server_->config.port));
        stream_.connect(results);
    }

    ~HttpSession() {
        beast::error_code ec;
        stream_.socket().shutdown(ip::tcp::socket::shutdown_both, ec);
    }

    std::string Post(const std::string &target, const std::string &body) {
        http::request<http::string_body> req{http::verb::post, target, 10};
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::content_length, std::to_string(body.size()));
        req.body() = body;
        http::write(stream_, req);
        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(stream_, buffer, res);
        return res.body();
    }

private:
    std::shared_ptr<MasterNode> server_;
    asio::io_context ioc_;
    beast::tcp_stream stream_;
};

class WebsocketSession {
public:
    WebsocketSession(asio::io_context &ioc, std::shared_ptr<MasterNode> server,
                     const std::string &target)
        : server_(std::move(server)), ws_(ioc) {
        ip::tcp::resolver resolver(ioc);
        auto results = resolver.resolve(server_->config.host, std::to_string(server_->config.port));
        connect(ws_.next_layer(), results.begin(), results.end());
        ws_.handshake(server_->config.host, target);
    }

    ~WebsocketSession() {
        ws_.close(websocket::close_code::normal);
    }

    void Write(const std::string &message) {
        ws_.write(asio::buffer(message));
    }

    void OnRead(auto handler) {
        ws_.template async_read(buffer_, [this, handler = std::move(handler)](
                                             const beast::error_code &ec, size_t bytes_written) {
            std::string message = beast::buffers_to_string(buffer_.data());
            handler(message);
            buffer_.clear();
            OnRead(handler);
        });
    }

private:
    std::shared_ptr<MasterNode> server_;
    websocket::stream<ip::tcp::socket> ws_;
    beast::flat_buffer buffer_;
};

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
