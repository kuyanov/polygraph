#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <string>
#include <thread>
#include <unordered_set>

#include <gtest/gtest.h>

#include "test_masternode.h"

const std::string kParseErrorPrefix = "Could not parse json:";
const std::string kValidationErrorPrefix = "Invalid document:";
const std::string kSemanticErrorPrefix = "Semantic error:";

void CheckSubmitStartsWith(const std::string &body, const std::string &prefix) {
    auto server = std::make_shared<MasterNode>();
    auto result = HttpSession(server).Post("/submit", body);
    ASSERT_TRUE(result.starts_with(prefix));
}

bool IsUuid(const std::string &s) {
    return s.size() == 36 && s[8] == '-' && s[13] == '-' && s[18] == '-' && s[23] == '-';
}

long long Timestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

void CheckGraphExecutionOrder(const UserGraph &graph, int cnt_users, int cnt_runners,
                              int expected_iterations) {
    auto server = std::make_shared<MasterNode>();
    std::string body = StringifyGraph(graph);
    auto uuid = HttpSession(server).Post("/submit", body);
    ASSERT_TRUE(IsUuid(uuid));

    std::vector<std::thread> runner_threads(cnt_runners);
    std::vector<asio::io_context> runner_contexts(cnt_runners);
    for (int runner_id = 0; runner_id < cnt_runners; ++runner_id) {
        auto &ioc = runner_contexts[runner_id];
        runner_threads[runner_id] = std::thread([&] {
            WebsocketSession session(ioc, server, "/runner/all");
            session.OnRead([&](const std::string &message) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                session.Write("ok");
            });
            ioc.run();
        });
    }

    std::condition_variable completed;
    std::atomic<int> cnt_users_connected = 0, cnt_users_completed = 0;
    std::vector<std::thread> user_threads(cnt_users);
    std::vector<asio::io_context> user_contexts(cnt_users);
    for (int user_id = 0; user_id < cnt_users; ++user_id) {
        auto &ioc = user_contexts[user_id];
        user_threads[user_id] = std::thread([&] {
            WebsocketSession session(ioc, server, "/graph/" + uuid);
            int cnt_blocks_completed = 0;
            session.OnRead([&](const std::string &message) {
                if (message == "complete") {
                    ++cnt_users_completed;
                    completed.notify_one();
                    ASSERT_EQ(cnt_blocks_completed, graph.blocks.size());
                } else {
                    ++cnt_blocks_completed;
                }
            });
            if (++cnt_users_connected == cnt_users) {
                session.Write("run");
            }
            ioc.run();
        });
    }

    std::mutex user_mutex;
    std::unique_lock<std::mutex> user_lock(user_mutex);
    auto start_time = Timestamp();
    completed.wait(user_lock, [&] {
        return cnt_users_completed == cnt_users;
    });
    auto end_time = Timestamp();
    ASSERT_TRUE(std::abs(end_time - start_time - 100 * expected_iterations) <= 50);

    for (int user_id = 0; user_id < cnt_users; ++user_id) {
        while (user_contexts[user_id].stopped()) {
        }
        user_contexts[user_id].stop();
        user_threads[user_id].join();
    }
    for (int runner_id = 0; runner_id < cnt_runners; ++runner_id) {
        while (runner_contexts[runner_id].stopped()) {
        }
        runner_contexts[runner_id].stop();
        runner_threads[runner_id].join();
    }
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
    auto server = std::make_shared<MasterNode>();
    std::string body = StringifyGraph({});
    std::unordered_set<std::string> uuids;
    for (int i = 0; i < 1000; i++) {
        auto uuid = HttpSession(server).Post("/submit", body);
        ASSERT_TRUE(IsUuid(uuid));
        uuids.insert(uuid);
    }
    ASSERT_EQ(uuids.size(), 1000);
}

TEST(Submit, MaxPayloadSize) {
    auto server = std::make_shared<MasterNode>();
    std::string body;
    body.resize(server->config.max_payload_size, '.');
    auto result = HttpSession(server).Post("/submit", body);
    ASSERT_TRUE(result.starts_with(kParseErrorPrefix));
    body.push_back('.');
    result = HttpSession(server).Post("/submit", body);
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
