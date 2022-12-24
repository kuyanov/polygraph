#include <limits.h>
#include <string>
#include <unordered_set>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "check.h"
#include "config.h"

using ::testing::IsEmpty;
using ::testing::MatchesRegex;
using ::testing::StartsWith;

const int kRunnerDelay = 500;
const Meta kGraphMeta = {"sample graph", "all", INT_MAX};

TEST(ParseError, Trivial) {
    EXPECT_THAT(Submit(""), StartsWith(errors::kParseErrorPrefix));
    EXPECT_THAT(Submit("{"), StartsWith(errors::kParseErrorPrefix));
    EXPECT_THAT(Submit("}"), StartsWith(errors::kParseErrorPrefix));
    EXPECT_THAT(Submit("{:}"), StartsWith(errors::kParseErrorPrefix));
    EXPECT_THAT(Submit("{,}"), StartsWith(errors::kParseErrorPrefix));
    EXPECT_THAT(Submit("{a:b}"), StartsWith(errors::kParseErrorPrefix));
    EXPECT_THAT(Submit("\"a\":\"b\""), StartsWith(errors::kParseErrorPrefix));
    EXPECT_THAT(Submit("{[]:[]}"), StartsWith(errors::kParseErrorPrefix));
    EXPECT_THAT(Submit("{\"a\":\"b}"), StartsWith(errors::kParseErrorPrefix));
    EXPECT_THAT(Submit("{\"a\":2,}"), StartsWith(errors::kParseErrorPrefix));
}

TEST(ValidationError, Trivial) {
    EXPECT_THAT(Submit("{}"), StartsWith(errors::kValidationErrorPrefix));
    EXPECT_THAT(Submit("[]"), StartsWith(errors::kValidationErrorPrefix));
}

TEST(ValidationError, MetaMissing) {
    EXPECT_THAT(Submit("{\"blocks\":[],\"connections\":[]}"),
                StartsWith(errors::kValidationErrorPrefix));
}

TEST(ValidationError, InvalidType) {
    EXPECT_THAT(
        Submit(
            "{\"blocks\":[],\"connections\":0,\"meta\":{\"name\":\"\",\"partition\":\"all\",\"max-"
            "runners\":1}}"),
        StartsWith(errors::kValidationErrorPrefix));
    EXPECT_THAT(
        Submit(
            "{\"blocks\":[],\"connections\":{},\"meta\":{\"name\":\"\",\"partition\":\"all\",\"max-"
            "runners\":1}}"),
        StartsWith(errors::kValidationErrorPrefix));
}

TEST(ValidationError, FilenameRegex) {
    for (const auto &filename : {"", ".", ".a", "a.", "a..b", "a b", "a/b"}) {
        EXPECT_THAT(SubmitGraph(Graph{{{"", {{filename}}}}, {}, kGraphMeta}),
                    StartsWith(errors::kValidationErrorPrefix));
    }
    for (const auto &filename : {"a", "a.in", "a.in.txt", "A.mp4", "a_b-c.DEF"}) {
        EXPECT_THAT(SubmitGraph(Graph{{{"", {{filename}}}}, {}, kGraphMeta}),
                    MatchesRegex(kUuidRegex));
    }
}

TEST(ValidationError, InvalidConnection) {
    EXPECT_THAT(SubmitGraph(Graph{{{}}, {{"regular", 0, 1, "a", "a"}}, kGraphMeta}),
                StartsWith(errors::kValidationErrorPrefix + errors::kInvalidConnection));
    EXPECT_THAT(SubmitGraph(Graph{{{}}, {{"regular", 1, 0, "a", "a"}}, kGraphMeta}),
                StartsWith(errors::kValidationErrorPrefix + errors::kInvalidConnection));
}

TEST(ValidationError, DuplicatedFilename) {
    EXPECT_THAT(
        SubmitGraph(Graph{
            {{}, {}, {}}, {{"regular", 0, 1, "a", "a"}, {"regular", 1, 2, "a", "a"}}, kGraphMeta}),
        StartsWith(errors::kValidationErrorPrefix + errors::kDuplicatedFilename));
    EXPECT_THAT(SubmitGraph(Graph{{{"", {{"a"}}}, {}}, {{"regular", 0, 1, "a", "a"}}, kGraphMeta}),
                StartsWith(errors::kValidationErrorPrefix + errors::kDuplicatedFilename));
    EXPECT_THAT(SubmitGraph(Graph{{{}, {"", {{"a"}}}}, {{"regular", 0, 1, "a", "a"}}, kGraphMeta}),
                StartsWith(errors::kValidationErrorPrefix + errors::kDuplicatedFilename));
}

TEST(ValidationError, Loop) {
    EXPECT_THAT(SubmitGraph(Graph{{{}}, {{"regular", 0, 0, "a", "a"}}, kGraphMeta}),
                StartsWith(errors::kValidationErrorPrefix + errors::kLoopsNotSupported));
}

TEST(Submit, GraphIdUnique) {
    std::string body = StringifyJSON(Serialize(Graph{{}, {}, kGraphMeta}));
    std::unordered_set<std::string> ids;
    for (int i = 0; i < 1000; i++) {
        auto id = Submit(body);
        EXPECT_THAT(id, MatchesRegex(kUuidRegex));
        ids.insert(id);
    }
    ASSERT_EQ(ids.size(), 1000);
}

TEST(Submit, MaxPayloadSize) {
    std::string body;
    body.resize(Config::Get().max_payload_size, '.');
    EXPECT_THAT(Submit(body), StartsWith(errors::kParseErrorPrefix));
    body.push_back('.');
    EXPECT_THAT(Submit(body), IsEmpty());
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
        {{}, {}, {}}, {{"regular", 0, 1, "a", "a"}, {"regular", 1, 2, "b", "b"}}, kGraphMeta};
    CheckGraphExecution(graph, 5, 1, 3, kRunnerDelay, 3 * kRunnerDelay);
    CheckGraphExecution(graph, 5, 10, 3, kRunnerDelay, 3 * kRunnerDelay);
}

TEST(Execution, Parallel) {
    Graph graph = {{{}, {}, {}}, {}, kGraphMeta};
    CheckGraphExecution(graph, 5, 1, 3, kRunnerDelay, 3 * kRunnerDelay);
    CheckGraphExecution(graph, 5, 3, 3, kRunnerDelay, kRunnerDelay);
}

TEST(Execution, MaxRunners) {
    Graph graph = {{{}, {}, {}, {}, {}, {}},
                   {{"regular", 0, 1, "a", "a"},
                    {"regular", 0, 2, "a", "a"},
                    {"regular", 0, 3, "a", "a"},
                    {"regular", 0, 4, "a", "a"},
                    {"regular", 1, 5, "b", "b1"},
                    {"regular", 2, 5, "b", "b2"},
                    {"regular", 3, 5, "b", "b3"},
                    {"regular", 4, 5, "b", "b4"}},
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
    Graph graph = {{{}, {}, {}, {}, {}, {}, {}},
                   {{"regular", 0, 1, "a", "a"},
                    {"regular", 1, 2, "b", "b"},
                    {"regular", 2, 3, "c", "c"},
                    {"regular", 3, 4, "d", "d"},
                    {"regular", 0, 5, "a", "f1"},
                    {"regular", 0, 5, "a", "f2"},
                    {"regular", 2, 5, "c", "f1"},
                    {"regular", 4, 5, "e", "f1"},
                    {"regular", 5, 6, "g", "g"},
                    {"regular", 6, 5, "f", "f2"}},
                   kGraphMeta};
    CheckGraphExecution(graph, 3, 2, 11, kRunnerDelay, 7 * kRunnerDelay);
}

TEST(Execution, FailedBlocks) {
    Graph graph = {
        {{}, {}, {}, {}, {}},
        {{"regular", 0, 1, "a", "a"}, {"regular", 2, 3, "b", "b"}, {"regular", 3, 4, "c", "c"}},
        kGraphMeta};
    CheckGraphExecution(graph, 3, 2, 3, kRunnerDelay, 2 * kRunnerDelay, {0, 3});
}

TEST(Execution, Stress) {
    std::vector<Block> blocks(100);
    Graph graph = {blocks, {}, kGraphMeta};
    for (int i = 0; i < 100; i++) {
        CheckGraphExecution(graph, 4, 4, 100, 0, -1);
    }
}
