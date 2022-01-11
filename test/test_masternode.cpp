#include <mutex>
#include <string>
#include <unordered_set>

#include <gtest/gtest.h>

#include "test_masternode.h"

const std::string kParseErrorPrefix = "Could not parse json:";
const std::string kValidationErrorPrefix = "Invalid document:";
const std::string kSemanticErrorPrefix = "Semantic error:";

void CheckSubmitStartsWith(const std::string &body, const std::string &prefix) {
    MasterNode server;
    auto result = server.Submit(body);
    ASSERT_TRUE(result.starts_with(prefix));
}

bool IsUuid(const std::string &s) {
    return s.size() == 36 && s[8] == '-' && s[13] == '-' && s[18] == '-' && s[23] == '-';
}

TEST(ParseError, Trivial) {
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

TEST(ValidationError, Trivial) {
    CheckSubmitStartsWith("{}", kValidationErrorPrefix);
    CheckSubmitStartsWith("[]", kValidationErrorPrefix);
}

TEST(ValidationError, ConnectionsMissing) {
    CheckSubmitStartsWith("{\"blocks\":[],\"meta\":{\"runner-group\":\"all\"}}",
                          kValidationErrorPrefix);
}

TEST(ValidationError, BlocksMissing) {
    CheckSubmitStartsWith("{\"connections\":[],\"meta\":{\"runner-group\":\"all\"}}",
                          kValidationErrorPrefix);
}

TEST(ValidationError, RunnerGroupMissing) {
    CheckSubmitStartsWith("{\"blocks\":[],\"connections\":[],\"meta\":{}}", kValidationErrorPrefix);
}

TEST(ValidationError, InvalidType) {
    CheckSubmitStartsWith("{\"blocks\":[],\"connections\":0,\"meta\":{\"runner-group\":\"all\"}}",
                          kValidationErrorPrefix);
    CheckSubmitStartsWith("{\"blocks\":[],\"connections\":{},\"meta\":{\"runner-group\":\"all\"}}",
                          kValidationErrorPrefix);
}

TEST(SemanticError, DuplicatedInput) {
    CheckSubmitStartsWith(StringifyGraph({{{"1", {{"a.in"}, {"a.in"}}, {}}}}),
                          kSemanticErrorPrefix + " Duplicated input name");
}

TEST(SemanticError, DuplicatedOutput) {
    CheckSubmitStartsWith(StringifyGraph({{{"1", {}, {{"a.out"}, {"a.out"}}}}}),
                          kSemanticErrorPrefix + " Duplicated output name");
}

TEST(SemanticError, ConnectionStartBlock) {
    CheckSubmitStartsWith(StringifyGraph({{{"1", {{"a.in"}}, {{"a.out"}}}}, {{1, 0, 0, 0}}}),
                          kSemanticErrorPrefix + " Invalid connection start block");
}

TEST(SemanticError, ConnectionStartOutput) {
    CheckSubmitStartsWith(StringifyGraph({{{"1", {{"a.in"}}, {{"a.out"}}}}, {{0, 1, 0, 0}}}),
                          kSemanticErrorPrefix + " Invalid connection start block output");
}

TEST(SemanticError, ConnectionEndBlock) {
    CheckSubmitStartsWith(StringifyGraph({{{"1", {{"a.in"}}, {{"a.out"}}}}, {{0, 0, -1, 0}}}),
                          kSemanticErrorPrefix + " Invalid connection end block");
}

TEST(SemanticError, ConnectionEndInput) {
    CheckSubmitStartsWith(StringifyGraph({{{"1", {{"a.in"}}, {{"a.out"}}}}, {{0, 0, 0, -1}}}),
                          kSemanticErrorPrefix + " Invalid connection end block input");
}

TEST(SemanticError, EndInputBindPath) {
    CheckSubmitStartsWith(
        StringifyGraph({{{"1", {{"a.in", ""}}, {{"a.out"}}}}, {{0, 0, 0, 0}}}),
        kSemanticErrorPrefix + " Connection end block input cannot have bind path");
}

TEST(Submit, GraphIdUnique) {
    MasterNode server;
    std::string body = StringifyGraph({});
    std::unordered_set<std::string> uuids;
    for (int i = 0; i < 1000; i++) {
        auto uuid = server.Submit(body);
        ASSERT_TRUE(IsUuid(uuid));
        uuids.insert(uuid);
    }
    ASSERT_EQ(uuids.size(), 1000);
}

TEST(Submit, MaxPayloadSize) {
    MasterNode server;
    std::string body;
    body.resize(server.config.max_payload_size, '.');
    auto result = server.Submit(body);
    ASSERT_TRUE(result.starts_with(kParseErrorPrefix));
    body.push_back('.');
    result = server.Submit(body);
    ASSERT_TRUE(result.empty());
}

TEST(ExecutionOrder, SingleBlock) {
    MasterNode server;
    std::string body = StringifyGraph({{{"1", {}, {}, {{"cmd"}}}}});
    auto uuid = server.Submit(body);
    ASSERT_TRUE(IsUuid(uuid));
    server.CreateRunner("all", [](const std::string &message) { ASSERT_EQ(message, "hello"); });

    std::mutex user_mutex;
    std::vector<std::string> order;
    server.CreateUser(uuid, [&](const std::string &message) {
        std::unique_lock<std::mutex> lock(user_mutex);
        order.push_back(message);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_EQ(order, std::vector<std::string>{"ok"});
}
