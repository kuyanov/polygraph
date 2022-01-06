#include <chrono>
#include <optional>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include <boost/beast.hpp>
#include <boost/lexical_cast.hpp>

#include <gtest/gtest.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include "run.h"

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace beast = boost::beast;
namespace http = beast::http;

class MasterNodeServer {
public:
    Config config;

    MasterNodeServer() : config("../masternode/config/test.json") {
        std::thread([*this] { Run(config); }).detach();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::string PostQuery(const std::string &target, const std::string &body) {
        asio::io_context ioc;
        beast::tcp_stream stream(ioc);
        ip::tcp::resolver resolver(ioc);

        auto results = resolver.resolve(config.host, std::to_string(config.port));
        stream.connect(results);

        http::request<http::string_body> req{http::verb::post, target, 10};
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::content_length, boost::lexical_cast<std::string>(body.size()));
        req.body() = body;

        http::write(stream, req);

        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(stream, buffer, res);

        return res.body();
    }
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
        std::string runner_group;
        int max_runners = 1;
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

bool IsUuid(const std::string &s) {
    return s.size() == 36 && s[8] == '-' && s[13] == '-' && s[18] == '-' && s[23] == '-';
}

void CheckSubmitStartsWith(const std::string &body, const std::string &prefix) {
    MasterNodeServer server;
    auto result = server.PostQuery("/submit", body);
    ASSERT_TRUE(result.starts_with(prefix));
}

const std::string kParseErrorPrefix = "Could not parse json:";
const std::string kValidationErrorPrefix = "Invalid document:";
const std::string kSemanticErrorPrefix = "Semantic error:";

TEST(TestMasterNode, GraphIdUnique) {
    MasterNodeServer server;
    std::string body = StringifyGraph({});
    std::unordered_set<std::string> results;
    for (int i = 0; i < 1000; i++) {
        auto result = server.PostQuery("/submit", body);
        ASSERT_TRUE(IsUuid(result));
        results.insert(result);
    }
    ASSERT_EQ(results.size(), 1000);
}

TEST(TestMasterNode, MaxPayloadSize) {
    MasterNodeServer server;
    std::string body;
    body.resize(server.config.max_payload_size, '.');
    auto result = server.PostQuery("/submit", body);
    ASSERT_TRUE(result.starts_with(kParseErrorPrefix));
    body.push_back('.');
    result = server.PostQuery("/submit", body);
    ASSERT_TRUE(result.empty());
}

TEST(TestMasterNode, ParseError) {
    CheckSubmitStartsWith("", kParseErrorPrefix);
    CheckSubmitStartsWith("{", kParseErrorPrefix);
    CheckSubmitStartsWith("}", kParseErrorPrefix);
    CheckSubmitStartsWith("{:}", kParseErrorPrefix);
    CheckSubmitStartsWith("{,}", kParseErrorPrefix);
    CheckSubmitStartsWith("{a:b}", kParseErrorPrefix);
    CheckSubmitStartsWith("\"a\":\"b\"", kParseErrorPrefix);
    CheckSubmitStartsWith("{[]:[]}", kParseErrorPrefix);
    CheckSubmitStartsWith("{\"a\":\"b}", kParseErrorPrefix);
    CheckSubmitStartsWith("{\"a\":2,}", kParseErrorPrefix);
}

TEST(TestMasterNode, ValidationError) {
    CheckSubmitStartsWith("{}", kValidationErrorPrefix);
    CheckSubmitStartsWith("[]", kValidationErrorPrefix);
    CheckSubmitStartsWith("{\"blocks\":[],\"meta\":{\"runner-group\":\"all\"}}",
                          kValidationErrorPrefix);
    CheckSubmitStartsWith("{\"connections\":[],\"meta\":{\"runner-group\":\"all\"}}",
                          kValidationErrorPrefix);
    CheckSubmitStartsWith("{\"blocks\":[],\"connections\":0,\"meta\":{\"runner-group\":\"all\"}}",
                          kValidationErrorPrefix);
    CheckSubmitStartsWith("{\"blocks\":[],\"connections\":{},\"meta\":{\"runner-group\":\"all\"}}",
                          kValidationErrorPrefix);
    CheckSubmitStartsWith("{\"blocks\":[],\"connections\":[],\"meta\":{}}", kValidationErrorPrefix);
}

TEST(TestMasterNode, SemanticErrorDuplicated) {
    CheckSubmitStartsWith(StringifyGraph({{{"1", {{"a.in"}, {"a.in"}}, {}}}}),
                          kSemanticErrorPrefix + " Duplicated input name");
    CheckSubmitStartsWith(StringifyGraph({{{"1", {}, {{"a.out"}, {"a.out"}}}}}),
                          kSemanticErrorPrefix + " Duplicated output name");
}

TEST(TestMasterNode, SemanticErrorConnection) {
    CheckSubmitStartsWith(StringifyGraph({{{"1", {{"a.in"}}, {{"a.out"}}}}, {{1, 0, 0, 0}}}),
                          kSemanticErrorPrefix + " Invalid connection start block");
    CheckSubmitStartsWith(StringifyGraph({{{"1", {{"a.in"}}, {{"a.out"}}}}, {{0, 1, 0, 0}}}),
                          kSemanticErrorPrefix + " Invalid connection start block output");
    CheckSubmitStartsWith(StringifyGraph({{{"1", {{"a.in"}}, {{"a.out"}}}}, {{0, 0, -1, 0}}}),
                          kSemanticErrorPrefix + " Invalid connection end block");
    CheckSubmitStartsWith(StringifyGraph({{{"1", {{"a.in"}}, {{"a.out"}}}}, {{0, 0, 0, -1}}}),
                          kSemanticErrorPrefix + " Invalid connection end block input");
}

TEST(TestMasterNode, SemanticErrorBindPath) {
    CheckSubmitStartsWith(
        StringifyGraph({{{"1", {{"a.in", ""}}, {{"a.out"}}}}, {{0, 0, 0, 0}}}),
        kSemanticErrorPrefix + " Connection end block input cannot have bind path");
}
