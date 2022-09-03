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
#include "operations.h"
#include "run_request.h"
#include "run_response.h"

namespace fs = std::filesystem;

const std::string kHost = Config::Get().host;
const int kPort = Config::Get().port;

static thread_local SchemaValidator request_validator("run_request.json");

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

void ImitateRun(const std::string &message, const Graph &graph, int runner_delay,
                const std::vector<size_t> &failed_blocks, std::string &response) {
    RunRequest run_request;
    Load(run_request, request_validator.ParseAndValidate(message));
    std::string container_name = run_request.container;
    size_t block_id = ParseBlockId(container_name);
    for (const auto &input : graph.blocks[block_id].inputs) {
        ASSERT_TRUE(fs::exists(fs::path(SANDBOX_DIR) / container_name / input.name));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(runner_delay));
    for (const auto &output : graph.blocks[block_id].outputs) {
        ASSERT_TRUE(!fs::exists(fs::path(SANDBOX_DIR) / container_name / output.name));
        std::ofstream(fs::path(SANDBOX_DIR) / container_name / output.name);
    }
    bool failed =
        std::find(failed_blocks.begin(), failed_blocks.end(), block_id) != failed_blocks.end();
    RunResponse run_response;
    run_response.has_error = failed && run_request.tasks.empty();
    for (const auto &task : run_request.tasks) {
        run_response.results.push_back({.exited = true, .exit_code = failed});
    }
    response = StringifyJSON(Dump(run_response));
}

void CheckGraphExecution(const Graph &graph, int cnt_clients, int cnt_runners, int exp_runs,
                         int runner_delay, int exp_delay,
                         const std::vector<size_t> &failed_blocks = {}) {
    std::string body = StringifyJSON(Dump(graph));
    auto uuid = HttpSession(kHost, kPort).Post("/submit", body);
    ASSERT_TRUE(IsUuid(uuid));

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
            session.Connect(kHost, kPort, "/graph/" + uuid);
            int cnt_blocks_completed = 0;
            session.OnRead([&](const std::string &message) {
                if (message == signals::kGraphComplete) {
                    if (++cnt_clients_completed == cnt_clients) {
                        completed.notify_one();
                    }
                    ASSERT_EQ(cnt_blocks_completed, exp_runs);
                } else {
                    ++cnt_blocks_completed;
                }
            });
            if (++cnt_clients_connected == cnt_clients) {
                session.Write(signals::kGraphRun);
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
