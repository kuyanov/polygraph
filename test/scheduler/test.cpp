#include <string>
#include <unordered_set>

#include "gtest/gtest.h"
#include "config.h"
#include "constants.h"
#include "execution.h"
#include "graph.h"

TEST(ParseError, Trivial) {
    CheckSubmitStartsWith("", errors::kParseErrorPrefix);
    CheckSubmitStartsWith("{", errors::kParseErrorPrefix);
    CheckSubmitStartsWith("}", errors::kParseErrorPrefix);
    CheckSubmitStartsWith("{:}", errors::kParseErrorPrefix);
    CheckSubmitStartsWith("{,}", errors::kParseErrorPrefix);
    CheckSubmitStartsWith("{a:b}", errors::kParseErrorPrefix);
    CheckSubmitStartsWith("\"a\":\"b\"", errors::kParseErrorPrefix);
    CheckSubmitStartsWith("{[]:[]}", errors::kParseErrorPrefix);
    CheckSubmitStartsWith("{\"a\":\"b}", errors::kParseErrorPrefix);
    CheckSubmitStartsWith("{\"a\":2,}", errors::kParseErrorPrefix);
}

TEST(ValidationError, Trivial) {
    CheckSubmitStartsWith("{}", errors::kValidationErrorPrefix);
    CheckSubmitStartsWith("[]", errors::kValidationErrorPrefix);
}

TEST(ValidationError, MetaMissing) {
    CheckSubmitStartsWith("{\"blocks\":[],\"connections\":[]}", errors::kValidationErrorPrefix);
}

TEST(ValidationError, InvalidType) {
    CheckSubmitStartsWith(
        "{\"blocks\":[],\"connections\":0,\"meta\":{\"name\":\"\",\"partition\":\"all\",\"max-"
        "runners\":1}}",
        errors::kValidationErrorPrefix);
    CheckSubmitStartsWith(
        "{\"blocks\":[],\"connections\":{},\"meta\":{\"name\":\"\",\"partition\":\"all\",\"max-"
        "runners\":1}}",
        errors::kValidationErrorPrefix);
}

TEST(ValidationError, DuplicatedFilename) {
    CheckSubmitStartsWith(StringifyGraph({{{"", {{"a.in"}, {"a.in"}}, {}}}}),
                          errors::kValidationErrorPrefix + errors::kDuplicatedFilename);
    CheckSubmitStartsWith(StringifyGraph({{{"", {}, {{"a.out"}, {"a.out"}}}}}),
                          errors::kValidationErrorPrefix + errors::kDuplicatedFilename);
    CheckSubmitStartsWith(StringifyGraph({{{"", {{"a.in"}, {"a"}}, {{"a.out"}, {"a"}}}}}),
                          errors::kValidationErrorPrefix + errors::kDuplicatedFilename);
}

TEST(ValidationError, InvalidFilename) {
    CheckSubmitStartsWith(StringifyGraph({{{"", {{}}, {}}}}),
                          errors::kValidationErrorPrefix + errors::kInvalidFilename);
    CheckSubmitStartsWith(StringifyGraph({{{"", {}, {{".."}}}}}),
                          errors::kValidationErrorPrefix + errors::kInvalidFilename);
    CheckSubmitStartsWith(StringifyGraph({{{"", {{"a/b"}}, {}}}}),
                          errors::kValidationErrorPrefix + errors::kInvalidFilename);
}

TEST(ValidationError, ConnectionStartBlock) {
    CheckSubmitStartsWith(
        StringifyGraph({{{"", {{"a.in"}}, {}}, {"", {}, {{"a.out"}}}}, {{2, 0, 0, 0}}}),
        errors::kValidationErrorPrefix + errors::kInvalidStartBlock);
}

TEST(ValidationError, ConnectionStartOutput) {
    CheckSubmitStartsWith(
        StringifyGraph({{{"", {{"a.in"}}, {}}, {"", {}, {{"a.out"}}}}, {{1, 1, 0, 0}}}),
        errors::kValidationErrorPrefix + errors::kInvalidStartBlockOutput);
}

TEST(ValidationError, ConnectionEndBlock) {
    CheckSubmitStartsWith(
        StringifyGraph({{{"", {{"a.in"}}, {}}, {"", {}, {{"a.out"}}}}, {{1, 0, -1, 0}}}),
        errors::kValidationErrorPrefix + errors::kInvalidEndBlock);
}

TEST(ValidationError, ConnectionEndInput) {
    CheckSubmitStartsWith(
        StringifyGraph({{{"", {{"a.in"}}, {}}, {"", {}, {{"a.out"}}}}, {{1, 0, 0, -1}}}),
        errors::kValidationErrorPrefix + errors::kInvalidEndBlockInput);
}

TEST(ValidationError, Loop) {
    CheckSubmitStartsWith(StringifyGraph({{{"", {{"a.in"}}, {{"a.out"}}}}, {{0, 0, 0, 0}}}),
                          errors::kValidationErrorPrefix + errors::kLoopsNotSupported);
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
    body.resize(Config::Instance().max_payload_size, '.');
    auto result = HttpSession(kHost, kPort).Post("/submit", body);
    ASSERT_TRUE(result.starts_with(errors::kParseErrorPrefix));
    body.push_back('.');
    result = HttpSession(kHost, kPort).Post("/submit", body);
    ASSERT_TRUE(result.empty());
}

TEST(Execution, Empty) {
    Graph graph = {};
    CheckGraphExecution(graph, 1, 1, 0, 100, 0);
}

TEST(Execution, SingleBlock) {
    Graph graph = {{{}}};
    CheckGraphExecution(graph, 5, 1, 1, 100, 100);
    CheckGraphExecution(graph, 5, 10, 1, 100, 100);
}

TEST(Execution, Bamboo) {
    Graph graph = {{{"0", {}, {{"a.out"}}}, {"1", {{"a.in"}}, {{"a.out"}}}, {"2", {{"a.in"}}, {}}},
                   {{0, 0, 1, 0}, {1, 0, 2, 0}}};
    CheckGraphExecution(graph, 5, 1, 3, 100, 300);
    CheckGraphExecution(graph, 5, 10, 3, 100, 300);
}

TEST(Execution, Parallel) {
    Graph graph = {{{"0"}, {"1"}, {"2"}}};
    CheckGraphExecution(graph, 5, 1, 3, 100, 300);
    CheckGraphExecution(graph, 5, 3, 3, 100, 100);
}

TEST(Execution, MaxRunners) {
    Graph graph = {{{"0", {}, {{"o1234"}}},
                    {"1", {{"i0"}}, {{"o5"}}},
                    {"2", {{"i0"}}, {{"o5"}}},
                    {"3", {{"i0"}}, {{"o5"}}},
                    {"4", {{"i0"}}, {{"o5"}}},
                    {"5", {{"i1"}, {"i2"}, {"i3"}, {"i4"}}, {}}},
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

TEST(Execution, FiniteCycle) {
    Graph graph = {{{"0", {}, {{"o155"}}},
                    {"1", {{"i0"}}, {{"o2"}}},
                    {"2", {{"i1"}}, {{"o3"}, {"o5"}}},
                    {"3", {{"i2"}}, {{"o4"}}},
                    {"4", {{"i3"}}, {{"o5"}}},
                    {"5", {{"i024"}, {"i06"}}, {{"o6"}}},
                    {"6", {{"i5"}}, {{"o5"}}}},
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

TEST(Execution, FailedBlocks) {
    Graph graph = {{{"0", {}, {{"o1"}}},
                    {"1", {{"i0"}}, {}},
                    {"2", {}, {{"o3"}}},
                    {"3", {{"i2"}}, {{"o4"}}},
                    {"4", {{"i3"}}, {}}},
                   {{0, 0, 1, 0}, {2, 0, 3, 0}, {3, 0, 4, 0}}};
    CheckGraphExecution(graph, 3, 2, 3, 100, 200, {0, 3});
}

TEST(Execution, Stress) {
    std::vector<Graph::Block> blocks(100);
    Graph graph = {blocks, {}};
    for (int i = 0; i < 100; i++) {
        CheckGraphExecution(graph, 4, 1, 100, 0, -1);
    }
}
