#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "config.h"
#include "constants.h"
#include "networking.h"
#include "user_graph.h"

const std::string kHost = Config::Instance().host;
const int kPort = Config::Instance().port;

void CheckSubmitStartsWith(const std::string &body, const std::string &prefix) {
    auto result = HttpSession(kHost, kPort).Post("/submit", body);
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

size_t ParseBlockId(const std::string &container_name) {
    size_t l = container_name.find('_');
    size_t r = container_name.find('_', l + 1);
    return std::stoul(container_name.substr(l + 1, r - l - 1));
}

void CheckGraphExecution(const UserGraph &graph, int cnt_users, int cnt_runners, int exp_runs,
                         int runner_delay, int exp_delay) {
    std::string body = StringifyGraph(graph);
    auto uuid = HttpSession(kHost, kPort).Post("/submit", body);
    ASSERT_TRUE(IsUuid(uuid));

    std::vector<std::thread> runner_threads(cnt_runners);
    std::vector<asio::io_context> runner_contexts(cnt_runners);
    for (int runner_id = 0; runner_id < cnt_runners; ++runner_id) {
        auto &ioc = runner_contexts[runner_id];
        runner_threads[runner_id] = std::thread([&] {
            WebsocketSession session(ioc, kHost, kPort, "/runner/" + graph.meta.runner_group);
            session.OnRead([&](const std::string &message) {
                std::string container_name = message;
                size_t block_id = ParseBlockId(container_name);
                for (const auto &input : graph.blocks[block_id].inputs) {
                    ASSERT_TRUE(std::filesystem::exists(filesystem::kSandboxPath / container_name /
                                                        input.name));
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(runner_delay));
                for (const auto &output : graph.blocks[block_id].outputs) {
                    ASSERT_TRUE(!std::filesystem::exists(filesystem::kSandboxPath / container_name /
                                                         output.name));
                    std::ofstream(filesystem::kSandboxPath / container_name / output.name)
                        << "content";
                }
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
            WebsocketSession session(ioc, kHost, kPort, "/graph/" + uuid);
            int cnt_blocks_completed = 0;
            session.OnRead([&](const std::string &message) {
                if (message == signals::kGraphComplete) {
                    if (++cnt_users_completed == cnt_users) {
                        completed.notify_one();
                    }
                    ASSERT_EQ(cnt_blocks_completed, exp_runs);
                } else {
                    ++cnt_blocks_completed;
                }
            });
            if (++cnt_users_connected == cnt_users) {
                session.Write(signals::kGraphRun);
            }
            ioc.run();
        });
    }

    std::mutex user_mutex;
    std::unique_lock<std::mutex> user_lock(user_mutex);
    auto start_time = Timestamp();
    completed.wait(user_lock, [&] { return cnt_users_completed == cnt_users; });
    auto end_time = Timestamp();
    if (exp_delay != -1) {
        auto error = end_time - start_time - exp_delay;
        ASSERT_TRUE(error >= 0 && error < runner_delay);
    }

    for (int user_id = 0; user_id < cnt_users; ++user_id) {
        user_contexts[user_id].stop();
        user_threads[user_id].join();
    }
    for (int runner_id = 0; runner_id < cnt_runners; ++runner_id) {
        runner_contexts[runner_id].stop();
        runner_threads[runner_id].join();
    }
}
