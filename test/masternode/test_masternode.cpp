#include <string>
#include <unordered_set>

#include <gtest/gtest.h>

#include "helper.h"
#include "networking.h"
#include "user_graph.h"

const std::string kParseErrorPrefix = "Could not parse json:";
const std::string kValidationErrorPrefix = "Invalid document:";
const std::string kSemanticErrorPrefix = "Semantic error:";

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
    CheckSubmitStartsWith(StringifyGraph({{{"", {{"a.in"}, {"a.in"}}, {}}}}),
                          kSemanticErrorPrefix + " Duplicated input name");
}

TEST(SemanticError, DuplicatedOutput) {
    CheckSubmitStartsWith(StringifyGraph({{{"", {}, {{"a.out"}, {"a.out"}}}}}),
                          kSemanticErrorPrefix + " Duplicated output name");
}

TEST(SemanticError, ConnectionStartBlock) {
    CheckSubmitStartsWith(StringifyGraph({{{"", {{"a.in"}}, {{"a.out"}}}}, {{1, 0, 0, 0}}}),
                          kSemanticErrorPrefix + " Invalid connection start block");
}

TEST(SemanticError, ConnectionStartOutput) {
    CheckSubmitStartsWith(StringifyGraph({{{"", {{"a.in"}}, {{"a.out"}}}}, {{0, 1, 0, 0}}}),
                          kSemanticErrorPrefix + " Invalid connection start block output");
}

TEST(SemanticError, ConnectionEndBlock) {
    CheckSubmitStartsWith(StringifyGraph({{{"", {{"a.in"}}, {{"a.out"}}}}, {{0, 0, -1, 0}}}),
                          kSemanticErrorPrefix + " Invalid connection end block");
}

TEST(SemanticError, ConnectionEndInput) {
    CheckSubmitStartsWith(StringifyGraph({{{"", {{"a.in"}}, {{"a.out"}}}}, {{0, 0, 0, -1}}}),
                          kSemanticErrorPrefix + " Invalid connection end block input");
}

TEST(SemanticError, EndInputBindPath) {
    CheckSubmitStartsWith(
        StringifyGraph({{{"", {{"a.in", ""}}, {{"a.out"}}}}, {{0, 0, 0, 0}}}),
        kSemanticErrorPrefix + " Connection end block input cannot have bind path");
}

TEST(Submit, GraphIdUnique) {
    std::string body = StringifyGraph({});
    std::unordered_set<std::string> uuids;
    for (int i = 0; i < 1000; i++) {
        auto uuid = HttpSession(kHost, kPort).Post("/submit", body);
        ASSERT_TRUE(IsUuid(uuid));
        uuids.insert(uuid);
    }
    ASSERT_EQ(uuids.size(), 1000);
}

TEST(Submit, MaxPayloadSize) {
    std::string body;
    body.resize(MasterNode::Instance().config.max_payload_size, '.');
    auto result = HttpSession(kHost, kPort).Post("/submit", body);
    ASSERT_TRUE(result.starts_with(kParseErrorPrefix));
    body.push_back('.');
    result = HttpSession(kHost, kPort).Post("/submit", body);
    ASSERT_TRUE(result.empty());
}

TEST(ExecutionOrder, Empty) {
    UserGraph graph = {};
    CheckGraphExecutionOrder(graph, 1, 1, 0);
}

TEST(ExecutionOrder, SingleBlock) {
    UserGraph graph = {{{}}};
    CheckGraphExecutionOrder(graph, 1, 1, 1);
    CheckGraphExecutionOrder(graph, 10, 1, 1);
    CheckGraphExecutionOrder(graph, 1, 10, 1);
    CheckGraphExecutionOrder(graph, 10, 10, 1);
}

TEST(ExecutionOrder, Bamboo) {
    UserGraph graph = {{{"0", {}, {{}}}, {"1", {{}}, {{}}}, {"2", {{}}, {{}}}},
                       {{0, 0, 1, 0}, {1, 0, 2, 0}}};
    CheckGraphExecutionOrder(graph, 1, 1, 3);
    CheckGraphExecutionOrder(graph, 10, 1, 3);
    CheckGraphExecutionOrder(graph, 1, 10, 3);
    CheckGraphExecutionOrder(graph, 10, 10, 3);
}

TEST(ExecutionOrder, Parallel) {
    UserGraph graph = {{{"0"}, {"1"}, {"2"}}};
    CheckGraphExecutionOrder(graph, 1, 1, 3);
    CheckGraphExecutionOrder(graph, 10, 1, 3);
    CheckGraphExecutionOrder(graph, 1, 3, 1);
    CheckGraphExecutionOrder(graph, 10, 3, 1);
}
