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

TEST(Execution, Empty) {
    UserGraph graph = {};
    CheckGraphExecution(graph, 1, 1, 0, 100, 0);
}

TEST(Execution, SingleBlock) {
    UserGraph graph = {{{}}};
    CheckGraphExecution(graph, 5, 1, 1, 100, 100);
    CheckGraphExecution(graph, 5, 10, 1, 100, 100);
}

TEST(Execution, Bamboo) {
    UserGraph graph = {{{"0", {}, {{}}}, {"1", {{}}, {{}}}, {"2", {{}}, {{}}}},
                       {{0, 0, 1, 0}, {1, 0, 2, 0}}};
    CheckGraphExecution(graph, 5, 1, 3, 100, 300);
    CheckGraphExecution(graph, 5, 10, 3, 100, 300);
}

TEST(Execution, Parallel) {
    UserGraph graph = {{{"0"}, {"1"}, {"2"}}};
    CheckGraphExecution(graph, 5, 1, 3, 100, 300);
    CheckGraphExecution(graph, 5, 3, 3, 100, 100);
}

TEST(Execution, MaxRunners) {
    UserGraph graph = {{{"0", {}, {{}}},
                        {"1", {{}}, {{}}},
                        {"2", {{}}, {{}}},
                        {"3", {{}}, {{}}},
                        {"4", {{}}, {{}}},
                        {"5", {{"1"}, {"2"}, {"3"}, {"4"}}}},
                       {{0, 0, 1, 0},
                        {0, 0, 2, 0},
                        {0, 0, 3, 0},
                        {0, 0, 4, 0},
                        {1, 0, 5, 0},
                        {2, 0, 5, 1},
                        {3, 0, 5, 2},
                        {4, 0, 5, 3}}};
    CheckGraphExecution(graph, 3, 1, 6, 100, 600);
    CheckGraphExecution(graph, 3, 2, 6, 100, 400);
    CheckGraphExecution(graph, 3, 3, 6, 100, 400);
    CheckGraphExecution(graph, 3, 4, 6, 100, 300);
    CheckGraphExecution(graph, 3, 6, 6, 100, 300);
    graph.meta.max_runners = 4;
    CheckGraphExecution(graph, 3, 4, 6, 100, 300);
    graph.meta.max_runners = 3;
    CheckGraphExecution(graph, 3, 4, 6, 100, 400);
    graph.meta.max_runners = 2;
    CheckGraphExecution(graph, 3, 4, 6, 100, 400);
    graph.meta.max_runners = 1;
    CheckGraphExecution(graph, 3, 4, 6, 100, 600);
}

TEST(Execution, FiniteLoop1) {
    UserGraph graph = {
        {{"0", {}, {{}}}, {"1", {{}}, {{}}}, {"2", {{}}, {{}}}, {"3", {{"0"}, {"1"}}, {{}}}},
        {{0, 0, 1, 0}, {1, 0, 2, 0}, {0, 0, 3, 0}, {0, 0, 3, 1}, {2, 0, 3, 1}, {3, 0, 3, 0}}};
    CheckGraphExecution(graph, 3, 2, 5, 100, 400);
}

TEST(Execution, FiniteLoop2) {
    UserGraph graph = {{{"0", {}, {{}}},
                        {"1", {{}}, {{}}},
                        {"2", {{}}, {{"0"}, {"1"}}},
                        {"3", {{}}, {{}}},
                        {"4", {{}}, {{}}},
                        {"5", {{"0"}, {"1"}}, {{}}},
                        {"6", {{}}, {{}}}},
                       {{0, 0, 1, 0},
                        {1, 0, 2, 0},
                        {2, 0, 3, 0},
                        {3, 0, 4, 0},
                        {0, 0, 5, 0},
                        {0, 0, 5, 1},
                        {2, 1, 5, 0},
                        {4, 0, 5, 0},
                        {5, 0, 6, 0},
                        {6, 0, 5, 1}}};
    CheckGraphExecution(graph, 3, 2, 11, 100, 700);
}

TEST(Execution, Stress) {
    std::vector<UserGraph::Block> blocks(100);
    UserGraph graph = {blocks, {}};
    for (int i = 0; i < 200; i++) {
        CheckGraphExecution(graph, 4, 1, 100, 0, -1);
    }
}
