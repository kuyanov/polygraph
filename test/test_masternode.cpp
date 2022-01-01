#include <chrono>
#include <string>
#include <thread>
#include <unordered_set>

#include <boost/beast.hpp>
#include <boost/lexical_cast.hpp>

#include <gtest/gtest.h>

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
    const int iter = 1000;
    std::string body = "{\"blocks\":[],\"connections\":[],\"meta\":{\"runner-group\":\"all\"}}";
    std::unordered_set<std::string> results;
    for (int i = 0; i < iter; i++) {
        auto result = server.PostQuery("/submit", body);
        ASSERT_TRUE(IsUuid(result));
        results.insert(result);
    }
    ASSERT_EQ(results.size(), iter);
}

TEST(TestMasterNode, SubmitMaxPayloadSize) {
    MasterNodeServer server;
    std::string body;
    body.resize(server.config.max_payload_size, '.');
    auto result = server.PostQuery("/submit", body);
    ASSERT_TRUE(result.starts_with(kParseErrorPrefix));
    body.push_back('.');
    result = server.PostQuery("/submit", body);
    ASSERT_TRUE(result.empty());
}

TEST(TestMasterNode, GraphParseError) {
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

TEST(TestMasterNode, GraphValidationError) {
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
    CheckSubmitStartsWith(
        "{\"blocks\":[{\"name\":\"block1\",\"inputs\":[{}],\"outputs\":[],\"tasks\":[]}],"
        "\"connections\":[],\"meta\":{\"runner-group\":\"all\"}}",
        kValidationErrorPrefix);
}

TEST(TestMasterNode, GraphSemanticErrorDuplicatedInput) {
    CheckSubmitStartsWith(
        "{\"blocks\":[{\"name\":\"1\",\"inputs\":[{\"name\":\"a.in\"},{\"name\":\"a.in\"}],"
        "\"outputs\":[],\"tasks\":[]}],\"connections\":[],"
        "\"meta\":{\"runner-group\":\"all\"}}",
        kSemanticErrorPrefix + " Duplicated input name");
}
TEST(TestMasterNode, GraphSemanticErrorDuplicatedOutput) {
    CheckSubmitStartsWith(
        "{\"blocks\":[{\"name\":\"1\",\"inputs\":[],\"outputs\":[{\"name\":\"a.out\"},"
        "{\"name\":\"a.out\"}],\"tasks\":[]}],\"connections\":[],"
        "\"meta\":{\"runner-group\":\"all\"}}",
        kSemanticErrorPrefix + " Duplicated output name");
}
TEST(TestMasterNode, GraphSemanticErrorStartBlock) {
    CheckSubmitStartsWith(
        "{\"blocks\":[{\"name\":\"1\",\"inputs\":[{\"name\":\"a.in\"}],"
        "\"outputs\":[{\"name\":\"a.out\"}],\"tasks\":[]}],"
        "\"connections\":[{\"start-block-id\":1,\"start-block-output\":\"a.out\","
        "\"end-block-id\":0,\"end-block-input\":\"a.in\"}],"
        "\"meta\":{\"runner-group\":\"all\"}}",
        kSemanticErrorPrefix + " Invalid connection start block");
}
TEST(TestMasterNode, GraphSemanticErrorEndBlock) {
    CheckSubmitStartsWith(
        "{\"blocks\":[{\"name\":\"1\",\"inputs\":[{\"name\":\"a.in\"}],"
        "\"outputs\":[{\"name\":\"a.out\"}],\"tasks\":[]}],"
        "\"connections\":[{\"start-block-id\":0,\"start-block-output\":\"a.out\","
        "\"end-block-id\":-1,\"end-block-input\":\"a.in\"}],"
        "\"meta\":{\"runner-group\":\"all\"}}",
        kSemanticErrorPrefix + " Invalid connection end block");
}
TEST(TestMasterNode, GraphSemanticErrorStartBlockOutput) {
    CheckSubmitStartsWith(
        "{\"blocks\":[{\"name\":\"1\",\"inputs\":[{\"name\":\"a.in\"}],"
        "\"outputs\":[{\"name\":\"a.out\"}],\"tasks\":[]}],"
        "\"connections\":[{\"start-block-id\":0,\"start-block-output\":\"b.out\","
        "\"end-block-id\":0,\"end-block-input\":\"a.in\"}],"
        "\"meta\":{\"runner-group\":\"all\"}}",
        kSemanticErrorPrefix + " Invalid connection start block output");
}
TEST(TestMasterNode, GraphSemanticErrorEndBlockInput) {
    CheckSubmitStartsWith(
        "{\"blocks\":[{\"name\":\"1\",\"inputs\":[{\"name\":\"a.in\"}],"
        "\"outputs\":[{\"name\":\"a.out\"}],\"tasks\":[]}],"
        "\"connections\":[{\"start-block-id\":0,\"start-block-output\":\"a.out\","
        "\"end-block-id\":0,\"end-block-input\":\"b.in\"}],"
        "\"meta\":{\"runner-group\":\"all\"}}",
        kSemanticErrorPrefix + " Invalid connection end block input");
}
