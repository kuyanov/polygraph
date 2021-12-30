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

const int kMaxPayloadSize = 16 * 1024 * 1024;

class MasterNodeServer {
public:
    MasterNodeServer()
        : config_{
              .host = "0.0.0.0",
              .port = 3000,
              .max_payload_size = kMaxPayloadSize,
              .graph_schema_file = "../masternode/schema/graph.json",
          } {
        std::thread([*this] { Run(config_); }).detach();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::string PostQuery(const std::string &target, const std::string &body) {
        asio::io_context ioc;
        beast::tcp_stream stream(ioc);
        ip::tcp::resolver resolver(ioc);

        auto results = resolver.resolve(config_.host, std::to_string(config_.port));
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

private:
    Config config_;
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

TEST(TestMasterNode, UuidUnique) {
    MasterNodeServer server;
//    const int iter = 1000;
//    std::string body = "{\"blocks\":[],\"connections\":[],\"files\":[]}";
//    std::unordered_set<std::string> results;
//    for (int i = 0; i < iter; i++) {
//        auto result = server.PostQuery("/submit", body);
//        ASSERT_TRUE(IsUuid(result));
//        results.insert(result);
//    }
//    ASSERT_EQ(results.size(), iter);
}

TEST(TestMasterNode, SubmitParseError) {
    MasterNodeServer server;
//    std::vector<std::string> bodies = {
//        "", "{", "}", "{:}", "{,}", "{a:b}", "\"a\":\"b\"", "{[]:[]}", "{\"a\":\"b}", "{\"a\":2,}"};
//    for (const auto &body : bodies) {
//        auto result = server.PostQuery("/submit", body);
//        ASSERT_TRUE(IsParseError(result));
//    }
}

TEST(TestMasterNode, SubmitValidationError) {
    MasterNodeServer server;
//    std::vector<std::string> bodies = {
//        "{}",
//        "[]",
//        "{\"blocks\":[],\"connections\":[]}",
//        "{\"connections\":[],\"files\":[]}",
//        "{\"blocks\":[],\"files\":[]}",
//        "{\"blocks\":[],\"connections\":[],\"files\":0}",
//        "{\"blocks\":[],\"connections\":[],\"files\":{}}",
//        "{\"blocks\":[{\"name\":\"block1\",\"inputs\":[{}],\"outputs\":[],\"tasks\":[]}],"
//        "\"connections\":[],\"files\":[]}"
//    };
//    for (const auto &body : bodies) {
//        auto result = server.PostQuery("/submit", body);
//        ASSERT_TRUE(IsValidationError(result));
//    }
}

TEST(TestMasterNode, SubmitLarge) {
    MasterNodeServer server;
//    std::string body;
//    body.resize(kMaxPayloadSize, '.');
//    auto result = server.PostQuery("/submit", body);
//    ASSERT_TRUE(IsParseError(result));
//    body.push_back('.');
//    result = server.PostQuery("/submit", body);
//    ASSERT_TRUE(result.empty());
}
