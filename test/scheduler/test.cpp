#include <string>
#include <unordered_set>

#include "gtest/gtest.h"
#include "config.h"
#include "constants.h"
#include "execution.h"
#include "test_graph.h"

const int kRunnerDelay = 500;

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
    CheckSubmitStartsWith(StringifyJSON(BuildGraph({{{{"a.in", "a.in"}, {}}}}).Dump()),
                          errors::kValidationErrorPrefix + errors::kDuplicatedFilename);
    CheckSubmitStartsWith(StringifyJSON(BuildGraph({{{{}, {"a.out", "a.out"}}}}).Dump()),
                          errors::kValidationErrorPrefix + errors::kDuplicatedFilename);
    CheckSubmitStartsWith(StringifyJSON(BuildGraph({{{{"a.in", "a"}, {"a.out", "a"}}}}).Dump()),
                          errors::kValidationErrorPrefix + errors::kDuplicatedFilename);
}

TEST(ValidationError, InvalidFilename) {
    CheckSubmitStartsWith(StringifyJSON(BuildGraph({{{{""}, {}}}}).Dump()),
                          errors::kValidationErrorPrefix + errors::kInvalidFilename);
    CheckSubmitStartsWith(StringifyJSON(BuildGraph({{{{}, {".."}}}}).Dump()),
                          errors::kValidationErrorPrefix + errors::kInvalidFilename);
    CheckSubmitStartsWith(StringifyJSON(BuildGraph({{{{"a/b"}, {}}}}).Dump()),
                          errors::kValidationErrorPrefix + errors::kInvalidFilename);
}

TEST(ValidationError, InvalidConnection) {
    CheckSubmitStartsWith(
        StringifyJSON(BuildGraph({{{{"a.in"}, {}}, {{}, {"a.out"}}}, {{2, 0, 0, 0}}}).Dump()),
        errors::kValidationErrorPrefix + errors::kInvalidConnection);
    CheckSubmitStartsWith(
        StringifyJSON(BuildGraph({{{{"a.in"}, {}}, {{}, {"a.out"}}}, {{1, 1, 0, 0}}}).Dump()),
        errors::kValidationErrorPrefix + errors::kInvalidConnection);
    CheckSubmitStartsWith(
        StringifyJSON(BuildGraph({{{{"a.in"}, {}}, {{}, {"a.out"}}}, {{1, 0, -1, 0}}}).Dump()),
        errors::kValidationErrorPrefix + errors::kInvalidConnection);
    CheckSubmitStartsWith(
        StringifyJSON(BuildGraph({{{{"a.in"}, {}}, {{}, {"a.out"}}}, {{1, 0, 0, -1}}}).Dump()),
        errors::kValidationErrorPrefix + errors::kInvalidConnection);
}

TEST(ValidationError, Loop) {
    CheckSubmitStartsWith(
        StringifyJSON(BuildGraph({{{{"a.in"}, {"a.out"}}}, {{0, 0, 0, 0}}}).Dump()),
        errors::kValidationErrorPrefix + errors::kLoopsNotSupported);
}

TEST(Submit, GraphIdUnique) {
    std::string body = StringifyJSON(BuildGraph({}).Dump());
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
    Graph graph = BuildGraph({});
    CheckGraphExecution(graph, 1, 1, 0, kRunnerDelay, 0);
}

TEST(Execution, SingleBlock) {
    Graph graph = BuildGraph({{{}}});
    CheckGraphExecution(graph, 5, 1, 1, kRunnerDelay, kRunnerDelay);
    CheckGraphExecution(graph, 5, 10, 1, kRunnerDelay, kRunnerDelay);
}

TEST(Execution, Bamboo) {
    Graph graph = BuildGraph(
        {{{{}, {"a.out"}}, {{"a.in"}, {"a.out"}}, {{"a.in"}, {}}}, {{0, 0, 1, 0}, {1, 0, 2, 0}}});
    CheckGraphExecution(graph, 5, 1, 3, kRunnerDelay, 3 * kRunnerDelay);
    CheckGraphExecution(graph, 5, 10, 3, kRunnerDelay, 3 * kRunnerDelay);
}

TEST(Execution, Parallel) {
    Graph graph = BuildGraph({{{}, {}, {}}});
    CheckGraphExecution(graph, 5, 1, 3, kRunnerDelay, 3 * kRunnerDelay);
    CheckGraphExecution(graph, 5, 3, 3, kRunnerDelay, kRunnerDelay);
}

TEST(Execution, MaxRunners) {
    Graph graph = BuildGraph({{{{}, {"o1234"}},
                               {{"i0"}, {"o5"}},
                               {{"i0"}, {"o5"}},
                               {{"i0"}, {"o5"}},
                               {{"i0"}, {"o5"}},
                               {{"i1", "i2", "i3", "i4"}, {}}},
                              {{0, 0, 1, 0},
                               {0, 0, 2, 0},
                               {0, 0, 3, 0},
                               {0, 0, 4, 0},
                               {1, 0, 5, 0},
                               {2, 0, 5, 1},
                               {3, 0, 5, 2},
                               {4, 0, 5, 3}}});
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
    Graph graph = BuildGraph({{{{}, {"o155"}},
                               {{"i0"}, {"o2"}},
                               {{"i1"}, {"o3", "o5"}},
                               {{"i2"}, {"o4"}},
                               {{"i3"}, {"o5"}},
                               {{"i024", "i06"}, {"o6"}},
                               {{"i5"}, {"o5"}}},
                              {{0, 0, 1, 0},
                               {1, 0, 2, 0},
                               {2, 0, 3, 0},
                               {3, 0, 4, 0},
                               {0, 0, 5, 0},
                               {0, 0, 5, 1},
                               {2, 1, 5, 0},
                               {4, 0, 5, 0},
                               {5, 0, 6, 0},
                               {6, 0, 5, 1}}});
    CheckGraphExecution(graph, 3, 2, 11, kRunnerDelay, 7 * kRunnerDelay);
}

TEST(Execution, FailedBlocks) {
    Graph graph =
        BuildGraph({{{{}, {"o1"}}, {{"i0"}, {}}, {{}, {"o3"}}, {{"i2"}, {"o4"}}, {{"i3"}, {}}},
                    {{0, 0, 1, 0}, {2, 0, 3, 0}, {3, 0, 4, 0}}});
    CheckGraphExecution(graph, 3, 2, 3, kRunnerDelay, 2 * kRunnerDelay, {0, 3});
}

TEST(Execution, Stress) {
    std::vector<TestBlock> blocks(100);
    Graph graph = BuildGraph({blocks});
    for (int i = 0; i < 100; i++) {
        CheckGraphExecution(graph, 4, 4, 100, 0, -1);
    }
}
