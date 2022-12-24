#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "config.h"
#include "constants.h"
#include "graph.h"
#include "json.h"
#include "net.h"
#include "result.h"
#include "serialize.h"
#include "task.h"
#include "uuid.h"

namespace fs = std::filesystem;

const std::string kHost = Config::Get().host;
const int kPort = Config::Get().port;

static thread_local SchemaValidator request_validator("run_request.json");

std::string Submit(const std::string &body) {
    return HttpSession(kHost, kPort).Post("/submit", body);
}

std::string SubmitGraph(const Graph &graph) {
    return Submit(StringifyJSON(Serialize(graph)));
}

long long Timestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

size_t ParseBlockId(const std::string &container) {
    size_t l = container.find('_');
    size_t r = container.find('_', l + 1);
    return std::stoul(container.substr(l + 1, r - l - 1));
}

void ImitateRun(const std::string &message, const Graph &graph, int runner_delay,
                const std::vector<size_t> &failed_blocks, std::string &response) {
    RunRequest run_request;
    Deserialize(run_request, request_validator.ParseAndValidate(message));
    std::string container = run_request.container;
    size_t block_id = ParseBlockId(container);
    for (const auto &bind : graph.blocks[block_id].binds) {
        ASSERT_TRUE(fs::exists(fs::path(SANDBOX_DIR) / container / bind.inside_filename));
    }
    for (const auto &connection : graph.connections) {
        if (connection.end_block_id == block_id) {
            ASSERT_TRUE(fs::exists(fs::path(SANDBOX_DIR) / container / connection.end_filename));
        }
    }
    for (const auto &connection : graph.connections) {
        if (connection.start_block_id == block_id) {
            ASSERT_TRUE(!fs::exists(fs::path(SANDBOX_DIR) / container / connection.start_filename));
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(runner_delay));
    for (const auto &connection : graph.connections) {
        if (connection.start_block_id == block_id) {
            std::ofstream(fs::path(SANDBOX_DIR) / container / connection.start_filename);
        }
    }
    bool failed =
        std::find(failed_blocks.begin(), failed_blocks.end(), block_id) != failed_blocks.end();
    RunResponse run_response;
    if (failed) {
        run_response.error = "Some error";
    } else {
        run_response.result = {.exited = true, .exit_code = 0};
    }
    response = StringifyJSON(Serialize(run_response));
}

void CheckGraphExecution(const Graph &graph, int cnt_clients, int cnt_runners, int exp_runs,
                         int runner_delay, int exp_delay,
                         const std::vector<size_t> &failed_blocks = {}) {
    std::string body = StringifyJSON(Serialize(graph));
    auto id = HttpSession(kHost, kPort).Post("/submit", body);
    EXPECT_THAT(id, ::testing::MatchesRegex(kUuidRegex));

    std::vector<std::thread> runner_threads(cnt_runners);
    std::vector<WebsocketClientSession> runner_sessions(cnt_runners);
    for (int runner_id = 0; runner_id < cnt_runners; ++runner_id) {
        auto &session = runner_sessions[runner_id];
        runner_threads[runner_id] = std::thread([&] {
            session.Connect(kHost, kPort, "/runner/" + graph.meta.partition);
            session.OnRead([&](const std::string &message) {
                std::string response;
                ImitateRun(message, graph, runner_delay, failed_blocks, response);
                session.Write(response);
            });
            session.Run();
        });
    }

    std::condition_variable completed;
    std::atomic<int> cnt_clients_connected = 0, cnt_clients_completed = 0;
    std::vector<std::thread> client_threads(cnt_clients);
    std::vector<WebsocketClientSession> client_sessions(cnt_clients);
    for (int client_id = 0; client_id < cnt_clients; ++client_id) {
        auto &session = client_sessions[client_id];
        client_threads[client_id] = std::thread([&] {
            session.Connect(kHost, kPort, "/graph/" + id);
            int cnt_blocks_completed = 0;
            session.OnRead([&](const std::string &message) {
                if (message == states::kComplete) {
                    if (++cnt_clients_completed == cnt_clients) {
                        completed.notify_one();
                    }
                    ASSERT_EQ(cnt_blocks_completed, exp_runs);
                } else {
                    BlockResponse block_response;
                    Deserialize(block_response, ParseJSON(message));
                    if (block_response.state == states::kComplete) {
                        ++cnt_blocks_completed;
                    }
                }
            });
            if (++cnt_clients_connected == cnt_clients) {
                session.Write(signals::kRun);
            }
            session.Run();
        });
    }

    std::mutex client_mutex;
    std::unique_lock<std::mutex> client_lock(client_mutex);
    auto start_time = Timestamp();
    completed.wait(client_lock, [&] { return cnt_clients_completed == cnt_clients; });
    auto end_time = Timestamp();
    if (exp_delay != -1) {
        auto error = end_time - start_time - exp_delay;
        ASSERT_GE(error, 0);
        ASSERT_LT(error, runner_delay);
    }

    for (int client_id = 0; client_id < cnt_clients; ++client_id) {
        client_sessions[client_id].Stop();
        client_threads[client_id].join();
    }
    for (int runner_id = 0; runner_id < cnt_runners; ++runner_id) {
        runner_sessions[runner_id].Stop();
        runner_threads[runner_id].join();
    }
}
