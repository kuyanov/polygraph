#include <climits>
#include <string>
#include <unordered_set>

#include "gtest/gtest.h"
#include "check.h"

const int kRunnerDelay = 500;
const Meta kWorkflowMeta = {"sample workflow", "all", INT_MAX};

TEST(ParseError, Trivial) {
    EXPECT_EQ(Submit("").status, SUBMIT_PARSE_ERROR);
    EXPECT_EQ(Submit("{").status, SUBMIT_PARSE_ERROR);
    EXPECT_EQ(Submit("}").status, SUBMIT_PARSE_ERROR);
    EXPECT_EQ(Submit("{:}").status, SUBMIT_PARSE_ERROR);
    EXPECT_EQ(Submit("{,}").status, SUBMIT_PARSE_ERROR);
    EXPECT_EQ(Submit("{a:b}").status, SUBMIT_PARSE_ERROR);
    EXPECT_EQ(Submit("\"a\":\"b\"").status, SUBMIT_PARSE_ERROR);
    EXPECT_EQ(Submit("{[]:[]}").status, SUBMIT_PARSE_ERROR);
    EXPECT_EQ(Submit("{\"a\":\"b}").status, SUBMIT_PARSE_ERROR);
    EXPECT_EQ(Submit("{\"a\":2,}").status, SUBMIT_PARSE_ERROR);
}

TEST(ValidationError, Trivial) {
    EXPECT_EQ(Submit("{}").status, SUBMIT_VALIDATION_ERROR);
    EXPECT_EQ(Submit("[]").status, SUBMIT_VALIDATION_ERROR);
}

TEST(ValidationError, MetaMissing) {
    EXPECT_EQ(Submit("{\"blocks\":[],\"connections\":[]}").status, SUBMIT_VALIDATION_ERROR);
}

TEST(ValidationError, InvalidType) {
    EXPECT_EQ(
        Submit(
            "{\"blocks\":[],\"connections\":0,\"meta\":{\"name\":\"\",\"partition\":\"all\",\"max-"
            "runners\":1}}")
            .status,
        SUBMIT_VALIDATION_ERROR);
    EXPECT_EQ(
        Submit(
            "{\"blocks\":[],\"connections\":{},\"meta\":{\"name\":\"\",\"partition\":\"all\",\"max-"
            "runners\":1}}")
            .status,
        SUBMIT_VALIDATION_ERROR);
}

TEST(ValidationError, InvalidConnection) {
    EXPECT_EQ(SubmitWorkflow({{{.outputs = {{"a"}}}}, {{0, 0, 1, 0}}, kWorkflowMeta}).data,
              INVALID_CONNECTION_ERROR);
    EXPECT_EQ(SubmitWorkflow({{{.inputs = {{"a", false}}}}, {{1, 0, 0, 0}}, kWorkflowMeta}).data,
              INVALID_CONNECTION_ERROR);
    EXPECT_EQ(
        SubmitWorkflow(
            {{{.outputs = {{"a"}}}, {.inputs = {{"a", false}}}}, {{0, 1, 1, 0}}, kWorkflowMeta})
            .data,
        INVALID_CONNECTION_ERROR);
    EXPECT_EQ(
        SubmitWorkflow(
            {{{.outputs = {{"a"}}}, {.inputs = {{"a", false}}}}, {{0, 0, 1, 1}}, kWorkflowMeta})
            .data,
        INVALID_CONNECTION_ERROR);
}

TEST(ValidationError, DuplicatedLocation) {
    EXPECT_EQ(
        SubmitWorkflow({{{.inputs = {{"a", false}}, .binds = {{"a", "a"}}}}, {}, kWorkflowMeta})
            .data,
        DUPLICATED_PATH_ERROR);
    EXPECT_EQ(
        SubmitWorkflow({{{.outputs = {{"a"}}, .binds = {{"a", "a"}}}}, {}, kWorkflowMeta}).data,
        DUPLICATED_PATH_ERROR);
}

TEST(Submit, WorkflowIdUnique) {
    std::unordered_set<std::string> ids;
    for (int i = 0; i < 1000; i++) {
        auto response = SubmitWorkflow(Workflow{{}, {}, kWorkflowMeta});
        EXPECT_EQ(response.status, SUBMIT_ACCEPTED);
        ids.insert(response.data);
    }
    ASSERT_EQ(ids.size(), 1000);
}

TEST(Submit, MaxPayloadLength) {
    std::string body;
    body.resize(TestSchedulerConfig::Get().max_payload_length, '.');
    EXPECT_EQ(Submit(body).status, SUBMIT_PARSE_ERROR);
    body.push_back('.');
    try {
        Submit(body);
        ASSERT_TRUE(false);
    } catch (const std::runtime_error &) {
    }
}

TEST(Execution, Empty) {
    Workflow workflow = {{}, {}, kWorkflowMeta};
    CheckExecution(workflow, 1, 1, 0, kRunnerDelay, 0);
}

TEST(Execution, SingleBlock) {
    Workflow workflow = {{{}}, {}, kWorkflowMeta};
    CheckExecution(workflow, 5, 1, 1, kRunnerDelay, kRunnerDelay);
    CheckExecution(workflow, 5, 10, 1, kRunnerDelay, kRunnerDelay);
}

TEST(Execution, Bamboo) {
    Workflow workflow = {{{.outputs = {{"a"}}},
                          {.inputs = {{"a", false}}, .outputs = {{"b"}}},
                          {.inputs = {{"b", false}}}},
                         {{0, 0, 1, 0}, {1, 0, 2, 0}},
                         kWorkflowMeta};
    CheckExecution(workflow, 5, 1, 3, kRunnerDelay, 3 * kRunnerDelay);
    CheckExecution(workflow, 5, 10, 3, kRunnerDelay, 3 * kRunnerDelay);
}

TEST(Execution, Parallel) {
    Workflow workflow = {{{}, {}, {}}, {}, kWorkflowMeta};
    CheckExecution(workflow, 5, 1, 3, kRunnerDelay, 3 * kRunnerDelay);
    CheckExecution(workflow, 5, 3, 3, kRunnerDelay, kRunnerDelay);
}

TEST(Execution, MaxRunners) {
    Workflow workflow = {{{.outputs = {{"a"}}},
                          {.inputs = {{"a", false}}, .outputs = {{"b"}}},
                          {.inputs = {{"a", false}}, .outputs = {{"b"}}},
                          {.inputs = {{"a", false}}, .outputs = {{"b"}}},
                          {.inputs = {{"a", false}}, .outputs = {{"b"}}},
                          {.inputs = {{"b1", false}, {"b2", false}, {"b3", false}, {"b4", false}}}},
                         {{0, 0, 1, 0},
                          {0, 0, 2, 0},
                          {0, 0, 3, 0},
                          {0, 0, 4, 0},
                          {1, 0, 5, 0},
                          {2, 0, 5, 1},
                          {3, 0, 5, 2},
                          {4, 0, 5, 3}},
                         kWorkflowMeta};
    CheckExecution(workflow, 3, 1, 6, kRunnerDelay, 6 * kRunnerDelay);
    CheckExecution(workflow, 3, 2, 6, kRunnerDelay, 4 * kRunnerDelay);
    CheckExecution(workflow, 3, 3, 6, kRunnerDelay, 4 * kRunnerDelay);
    CheckExecution(workflow, 3, 4, 6, kRunnerDelay, 3 * kRunnerDelay);
    CheckExecution(workflow, 3, 6, 6, kRunnerDelay, 3 * kRunnerDelay);
    workflow.meta.max_runners = 4;
    CheckExecution(workflow, 3, 4, 6, kRunnerDelay, 3 * kRunnerDelay);
    workflow.meta.max_runners = 3;
    CheckExecution(workflow, 3, 4, 6, kRunnerDelay, 4 * kRunnerDelay);
    workflow.meta.max_runners = 2;
    CheckExecution(workflow, 3, 4, 6, kRunnerDelay, 4 * kRunnerDelay);
    workflow.meta.max_runners = 1;
    CheckExecution(workflow, 3, 4, 6, kRunnerDelay, 6 * kRunnerDelay);
}

TEST(Execution, FiniteLoop) {
    Workflow workflow = {{{.outputs = {{"a"}}},
                          {.inputs = {{"a", false}}, .outputs = {{"b"}}},
                          {.inputs = {{"b", false}}, .outputs = {{"c"}}},
                          {.inputs = {{"c", false}}, .outputs = {{"d"}}},
                          {.inputs = {{"d", false}}, .outputs = {{"e"}}},
                          {.inputs = {{"e1", false}, {"e2", false}}, .outputs = {{"e"}}}},
                         {{0, 0, 1, 0},
                          {1, 0, 2, 0},
                          {2, 0, 3, 0},
                          {3, 0, 4, 0},
                          {0, 0, 5, 0},
                          {0, 0, 5, 1},
                          {2, 0, 5, 0},
                          {4, 0, 5, 0},
                          {5, 0, 5, 1}},
                         kWorkflowMeta};
    CheckExecution(workflow, 3, 2, 8, kRunnerDelay, 6 * kRunnerDelay);
}

TEST(Execution, FiniteCycle) {
    Workflow workflow = {{{.outputs = {{"a"}}},
                          {.inputs = {{"a", false}}, .outputs = {{"b"}}},
                          {.inputs = {{"b", false}}, .outputs = {{"c"}}},
                          {.inputs = {{"c", false}}, .outputs = {{"d"}}},
                          {.inputs = {{"d", false}}, .outputs = {{"e"}}},
                          {.inputs = {{"e1", false}, {"e2", false}}, .outputs = {{"f"}}},
                          {.inputs = {{"f", false}}, .outputs = {{"e"}}}},
                         {{0, 0, 1, 0},
                          {1, 0, 2, 0},
                          {2, 0, 3, 0},
                          {3, 0, 4, 0},
                          {0, 0, 5, 0},
                          {0, 0, 5, 1},
                          {2, 0, 5, 0},
                          {4, 0, 5, 0},
                          {5, 0, 6, 0},
                          {6, 0, 5, 1}},
                         kWorkflowMeta};
    CheckExecution(workflow, 3, 2, 11, kRunnerDelay, 7 * kRunnerDelay);
}

TEST(Execution, CachedInputs) {
    Workflow workflow = {{{.outputs = {{"a"}}},
                          {.inputs = {{"a", false}}, .outputs = {{"b"}}},
                          {.inputs = {{"b", false}}, .outputs = {{"c"}}},
                          {.inputs = {{"c", false}}, .outputs = {{"d"}}},
                          {.inputs = {{"d", false}}, .outputs = {{"e"}}},
                          {.inputs = {{"e1", false}, {"e2", true}}, .outputs = {{"f"}}},
                          {.inputs = {{"f1", false}, {"f2", true}}}},
                         {{0, 0, 1, 0},
                          {1, 0, 2, 0},
                          {2, 0, 3, 0},
                          {3, 0, 4, 0},
                          {0, 0, 5, 0},
                          {0, 0, 5, 1},
                          {0, 0, 6, 1},
                          {2, 0, 5, 0},
                          {4, 0, 5, 0},
                          {5, 0, 6, 0}},
                         kWorkflowMeta};
    CheckExecution(workflow, 3, 2, 11, kRunnerDelay, 7 * kRunnerDelay);
}

TEST(Execution, FailedBlocks) {
    Workflow workflow = {{{.outputs = {{"a"}}},
                          {.inputs = {{"a", false}}},
                          {.outputs = {{"b"}}},
                          {.inputs = {{"b", false}}, .outputs = {{"c"}}},
                          {.inputs = {{"c", false}}}},
                         {{0, 0, 1, 0}, {2, 0, 3, 0}, {3, 0, 4, 0}},
                         kWorkflowMeta};
    CheckExecution(workflow, 3, 2, 3, kRunnerDelay, 2 * kRunnerDelay, {0, 3});
}

TEST(Execution, Stress) {
    std::vector<Block> blocks(100);
    Workflow workflow = {blocks, {}, kWorkflowMeta};
    for (int i = 0; i < 100; i++) {
        CheckExecution(workflow, 4, 4, 100, 0, -1);
    }
}
