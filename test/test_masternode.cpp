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
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::string PostQuery(const std::string &target, const std::string &body) {
        asio::io_context ioc;
        beast::tcp_stream stream(ioc);
        ip::tcp::resolver resolver(ioc);

        auto results = resolver.resolve(config.host, std::to_string(config.port));
        stream.connect(results);
        http::request<http::string_body> req{http::verb::post, target, 10};
        req.set(http::field::host, "localhost");
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

bool IsParseError(const std::string &s) {
    return s.starts_with("Could not parse json:");
}

bool IsValidationError(const std::string &s) {
    return s.starts_with("Invalid document:");
}

bool IsSemanticError(const std::string &s) {
    return s.starts_with("Semantic error:");
}

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

TEST(TestMasterNode, SubmitLarge) {
    MasterNodeServer server;
    std::string body;
    body.resize(server.config.max_payload_size, '.');
    auto result = server.PostQuery("/submit", body);
    ASSERT_TRUE(IsParseError(result));
    body.push_back('.');
    result = server.PostQuery("/submit", body);
    ASSERT_TRUE(result.empty());
}

TEST(TestMasterNode, GraphParseError) {
    MasterNodeServer server;
    std::vector<std::string> bodies = {
        "", "{", "}", "{:}", "{,}", "{a:b}", "\"a\":\"b\"", "{[]:[]}", "{\"a\":\"b}", "{\"a\":2,}"};
    for (const auto &body : bodies) {
        auto result = server.PostQuery("/submit", body);
        ASSERT_TRUE(IsParseError(result));
    }
}

TEST(TestMasterNode, GraphValidationError) {
    MasterNodeServer server;
    std::vector<std::string> bodies = {
        "{}",
        "[]",
        "{\"blocks\":[],\"connections\":[]}",
        "{\"connections\":[],\"files\":[]}",
        "{\"blocks\":[],\"files\":[]}",
        "{\"blocks\":[],\"connections\":[],\"files\":0}",
        "{\"blocks\":[],\"connections\":[],\"files\":{}}",
        "{\"blocks\":[{\"name\":\"block1\",\"inputs\":[{}],\"outputs\":[],\"tasks\":[]}],"
        "\"connections\":[],\"files\":[]}"};
    for (const auto &body : bodies) {
        auto result = server.PostQuery("/submit", body);
        ASSERT_TRUE(IsValidationError(result));
    }
}

TEST(TestMasterNode, GraphSemanticError) {
    MasterNodeServer server;
    // TODO
}
