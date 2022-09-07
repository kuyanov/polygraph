#include <limits.h>
#include <string>
#include <unordered_set>

#include "gtest/gtest.h"
#include "check.h"
#include "config.h"

const int kRunnerDelay = 500;
const Meta kGraphMeta = {"sample graph", "all", INT_MAX};

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
    CheckSubmitStartsWith(
        StringifyJSON(Dump(Graph{{{"", {}, {{"a.in"}, {"a.in"}}, {}}}, {}, kGraphMeta})),
        errors::kValidationErrorPrefix + errors::kDuplicatedFilename);
    CheckSubmitStartsWith(
        StringifyJSON(Dump(Graph{{{"", {}, {}, {{"a.out"}, {"a.out"}}}}, {}, kGraphMeta})),
        errors::kValidationErrorPrefix + errors::kDuplicatedFilename);
    CheckSubmitStartsWith(
        StringifyJSON(
            Dump(Graph{{{"", {{"a"}, {"a.out"}}, {{"a.in"}}, {{"a.out"}}}}, {}, kGraphMeta})),
        errors::kValidationErrorPrefix + errors::kDuplicatedFilename);
}

TEST(ValidationError, InvalidFilename) {
    CheckSubmitStartsWith(StringifyJSON(Dump(Graph{{{"", {{}}, {}, {}}}, {}, kGraphMeta})),
                          errors::kValidationErrorPrefix + errors::kInvalidFilename);
    CheckSubmitStartsWith(StringifyJSON(Dump(Graph{{{"", {}, {{".."}}, {}}}, {}, kGraphMeta})),
                          errors::kValidationErrorPrefix + errors::kInvalidFilename);
    CheckSubmitStartsWith(StringifyJSON(Dump(Graph{{{"", {}, {}, {{"a/b"}}}}, {}, kGraphMeta})),
                          errors::kValidationErrorPrefix + errors::kInvalidFilename);
}

TEST(ValidationError, InvalidPermissions) {
    CheckSubmitStartsWith(
        StringifyJSON(Dump(Graph{{{"", {{"a", "a", -1}}, {}, {}}}, {}, kGraphMeta})),
        errors::kValidationErrorPrefix + errors::kInvalidPermissions);
    CheckSubmitStartsWith(
        StringifyJSON(Dump(Graph{{{"", {{"a", "a", 8}}, {}, {}}}, {}, kGraphMeta})),
        errors::kValidationErrorPrefix + errors::kInvalidPermissions);
}

TEST(ValidationError, InvalidConnection) {
    CheckSubmitStartsWith(
        StringifyJSON(Dump(Graph{
            {{"", {}, {{"a.in"}}, {}}, {"", {}, {}, {{"a.out"}}}}, {{2, 0, 0, 0}}, kGraphMeta})),
        errors::kValidationErrorPrefix + errors::kInvalidConnection);
    CheckSubmitStartsWith(
        StringifyJSON(Dump(Graph{
            {{"", {}, {{"a.in"}}, {}}, {"", {}, {}, {{"a.out"}}}}, {{1, 1, 0, 0}}, kGraphMeta})),
        errors::kValidationErrorPrefix + errors::kInvalidConnection);
    CheckSubmitStartsWith(
        StringifyJSON(Dump(Graph{
            {{"", {}, {{"a.in"}}, {}}, {"", {}, {}, {{"a.out"}}}}, {{1, 0, -1, 0}}, kGraphMeta})),
        errors::kValidationErrorPrefix + errors::kInvalidConnection);
    CheckSubmitStartsWith(
        StringifyJSON(Dump(Graph{
            {{"", {}, {{"a.in"}}, {}}, {"", {}, {}, {{"a.out"}}}}, {{1, 0, 0, -1}}, kGraphMeta})),
        errors::kValidationErrorPrefix + errors::kInvalidConnection);
}

TEST(ValidationError, Loop) {
    CheckSubmitStartsWith(
        StringifyJSON(Dump(Graph{{{"", {}, {{"a.in"}}, {{"a.out"}}}}, {{0, 0, 0, 0}}, kGraphMeta})),
        errors::kValidationErrorPrefix + errors::kLoopsNotSupported);
}

TEST(Submit, GraphIdUnique) {
    std::string body = StringifyJSON(Dump(Graph{{}, {}, kGraphMeta}));
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
    body.resize(Config::Get().max_payload_size, '.');
    auto result = HttpSession(kHost, kPort).Post("/submit", body);
    ASSERT_TRUE(result.starts_with(errors::kParseErrorPrefix));
    body.push_back('.');
    result = HttpSession(kHost, kPort).Post("/submit", body);
    ASSERT_TRUE(result.empty());
}

TEST(Execution, Empty) {
    Graph graph = {{}, {}, kGraphMeta};
    CheckGraphExecution(graph, 1, 1, 0, kRunnerDelay, 0);
}

TEST(Execution, SingleBlock) {
    Graph graph = {{{}}, {}, kGraphMeta};
    CheckGraphExecution(graph, 5, 1, 1, kRunnerDelay, kRunnerDelay);
    CheckGraphExecution(graph, 5, 10, 1, kRunnerDelay, kRunnerDelay);
}

TEST(Execution, Bamboo) {
    Graph graph = {
        {{"", {}, {}, {{"a.out"}}}, {"", {}, {{"a.in"}}, {{"a.out"}}}, {"", {}, {{"a.in"}}, {}}},
        {{0, 0, 1, 0}, {1, 0, 2, 0}},
        kGraphMeta};
    CheckGraphExecution(graph, 5, 1, 3, kRunnerDelay, 3 * kRunnerDelay);
    CheckGraphExecution(graph, 5, 10, 3, kRunnerDelay, 3 * kRunnerDelay);
}

TEST(Execution, Parallel) {
    Graph graph = {{{}, {}, {}}, {}, kGraphMeta};
    CheckGraphExecution(graph, 5, 1, 3, kRunnerDelay, 3 * kRunnerDelay);
    CheckGraphExecution(graph, 5, 3, 3, kRunnerDelay, kRunnerDelay);
}

TEST(Execution, MaxRunners) {
    Graph graph = {{{"0", {}, {}, {{"o1234"}}},
                    {"1", {}, {{"i0"}}, {{"o5"}}},
                    {"2", {}, {{"i0"}}, {{"o5"}}},
                    {"3", {}, {{"i0"}}, {{"o5"}}},
                    {"4", {}, {{"i0"}}, {{"o5"}}},
                    {"5", {}, {{"i1"}, {"i2"}, {"i3"}, {"i4"}}, {}}},
                   {{0, 0, 1, 0},
                    {0, 0, 2, 0},
                    {0, 0, 3, 0},
                    {0, 0, 4, 0},
                    {1, 0, 5, 0},
                    {2, 0, 5, 1},
                    {3, 0, 5, 2},
                    {4, 0, 5, 3}},
                   kGraphMeta};
    CheckGraphExecution(graph, 3, 1, 6, kRunnerDelay, 6 * kRunnerDelay);
    CheckGraphExecution(graph, 3, 2, 6, kRunnerDelay, 4 * kRunnerDelay);
    CheckGraphExecution(graph, 3, 3, 6, kRunnerDelay, 4 * kRunnerDelay);
    CheckGraphExecution(graph, 3, 4, 6, kRunnerDelay, 3 * kRunnerDelay);
    CheckGraphExecution(graph, 3, 6, 6, kRunnerDelay, 3 * kRunnerDelay);
    graph.meta.max_runners = 4;
    CheckGraphExecution(graph, 3, 4, 6, kRunnerDelay, 3 * kRunnerDelay);
    graph.meta.max_runners = 3;
    CheckGraphExecution(graph, 3, 4, 6, kRunnerDelay, 4 * kRunnerDelay);
    graph.meta.max_runners = 2;
    CheckGraphExecution(graph, 3, 4, 6, kRunnerDelay, 4 * kRunnerDelay);
    graph.meta.max_runners = 1;
    CheckGraphExecution(graph, 3, 4, 6, kRunnerDelay, 6 * kRunnerDelay);
}

TEST(Execution, FiniteCycle) {
    Graph graph = {{{"0", {}, {}, {{"o155"}}},
                    {"1", {}, {{"i0"}}, {{"o2"}}},
                    {"2", {}, {{"i1"}}, {{"o3"}, {"o5"}}},
                    {"3", {}, {{"i2"}}, {{"o4"}}},
                    {"4", {}, {{"i3"}}, {{"o5"}}},
                    {"5", {}, {{"i024"}, {"i06"}}, {{"o6"}}},
                    {"6", {}, {{"i5"}}, {{"o5"}}}},
                   {{0, 0, 1, 0},
                    {1, 0, 2, 0},
                    {2, 0, 3, 0},
                    {3, 0, 4, 0},
                    {0, 0, 5, 0},
                    {0, 0, 5, 1},
                    {2, 1, 5, 0},
                    {4, 0, 5, 0},
                    {5, 0, 6, 0},
                    {6, 0, 5, 1}},
                   kGraphMeta};
    CheckGraphExecution(graph, 3, 2, 11, kRunnerDelay, 7 * kRunnerDelay);
}

TEST(Execution, FailedBlocks) {
    Graph graph = {{{"0", {}, {}, {{"o1"}}, {{}}},
                    {"1", {}, {{"i0"}}, {}},
                    {"2", {}, {}, {{"o3"}}},
                    {"3", {}, {{"i2"}}, {{"o4"}}},
                    {"4", {}, {{"i3"}}, {}}},
                   {{0, 0, 1, 0}, {2, 0, 3, 0}, {3, 0, 4, 0}},
                   kGraphMeta};
    CheckGraphExecution(graph, 3, 2, 3, kRunnerDelay, 2 * kRunnerDelay, {0, 3});
}

TEST(Execution, FilesystemError) {
    Graph graph = {{{"0", {{"a", "a", 7}}, {}, {}},
                    {"1", {}, {}, {{"o2"}}},
                    {"2", {{"a", "a", 7}}, {{"i1"}}, {}}},
                   {{1, 0, 2, 0}},
                   kGraphMeta};
    CheckGraphExecution(graph, 3, 1, 3, kRunnerDelay, kRunnerDelay);
    CheckGraphExecution(graph, 3, 2, 3, kRunnerDelay, kRunnerDelay);
}

TEST(Execution, Stress) {
    std::vector<Block> blocks(100);
    Graph graph = {{blocks}, {}, kGraphMeta};
    for (int i = 0; i < 100; i++) {
        CheckGraphExecution(graph, 4, 4, 100, 0, -1);
    }
}
