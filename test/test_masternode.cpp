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

const char *parse_error_prefix = "Could not parse json:";
const char *validation_error_prefix = "Invalid document:";
const char *semantic_error_prefix = "Semantic error:";

TEST(TestMasterNode, GraphIdUnique) {
    MasterNodeServer server;
    const int iter = 1000;
    std::string body = "{\"blocks\":[],\"connections\":[],\"files\":[]}";
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
    ASSERT_TRUE(result.starts_with(parse_error_prefix));
    body.push_back('.');
    result = server.PostQuery("/submit", body);
    ASSERT_TRUE(result.empty());
}

TEST(TestMasterNode, GraphParseError1) {
    CheckSubmitStartsWith("", parse_error_prefix);
}
TEST(TestMasterNode, GraphParseError2) {
    CheckSubmitStartsWith("{", parse_error_prefix);
}
TEST(TestMasterNode, GraphParseError3) {
    CheckSubmitStartsWith("}", parse_error_prefix);
}
TEST(TestMasterNode, GraphParseError4) {
    CheckSubmitStartsWith("{:}", parse_error_prefix);
}
TEST(TestMasterNode, GraphParseError5) {
    CheckSubmitStartsWith("{,}", parse_error_prefix);
}
TEST(TestMasterNode, GraphParseError6) {
    CheckSubmitStartsWith("{a:b}", parse_error_prefix);
}
TEST(TestMasterNode, GraphParseError7) {
    CheckSubmitStartsWith("\"a\":\"b\"", parse_error_prefix);
}
TEST(TestMasterNode, GraphParseError8) {
    CheckSubmitStartsWith("{[]:[]}", parse_error_prefix);
}
TEST(TestMasterNode, GraphParseError9) {
    CheckSubmitStartsWith("{\"a\":\"b}", parse_error_prefix);
}
TEST(TestMasterNode, GraphParseError10) {
    CheckSubmitStartsWith("{\"a\":2,}", parse_error_prefix);
}

TEST(TestMasterNode, GraphValidationError1) {
    CheckSubmitStartsWith("{}", validation_error_prefix);
}
TEST(TestMasterNode, GraphValidationError2) {
    CheckSubmitStartsWith("[]", validation_error_prefix);
}
TEST(TestMasterNode, GraphValidationError3) {
    CheckSubmitStartsWith("{\"blocks\":[],\"connections\":[]}", validation_error_prefix);
}
TEST(TestMasterNode, GraphValidationError4) {
    CheckSubmitStartsWith("{\"connections\":[],\"files\":[]}", validation_error_prefix);
}
TEST(TestMasterNode, GraphValidationError5) {
    CheckSubmitStartsWith("{\"blocks\":[],\"files\":[]}", validation_error_prefix);
}
TEST(TestMasterNode, GraphValidationError6) {
    CheckSubmitStartsWith("{\"blocks\":[],\"connections\":[],\"files\":0}",
                          validation_error_prefix);
}
TEST(TestMasterNode, GraphValidationError7) {
    CheckSubmitStartsWith("{\"blocks\":[],\"connections\":[],\"files\":{}}",
                          validation_error_prefix);
}
TEST(TestMasterNode, GraphValidationError8) {
    CheckSubmitStartsWith(
        "{\"blocks\":[{\"name\":\"block1\",\"inputs\":[{}],\"outputs\":[],\"tasks\":[]}],"
        "\"connections\":[],\"files\":[]}",
        validation_error_prefix);
}

TEST(TestMasterNode, GraphSemanticError1) {
    // TODO
}
