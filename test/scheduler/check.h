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
#include "constants.h"
#include "json.h"
#include "net.h"
#include "serialization/all.h"
#include "structures/all.h"
#include "uuid.h"

namespace fs = std::filesystem;

const std::string kHost = "127.0.0.1";
const int kPort = 3000;

std::string Submit(const std::string &body) {
    return HttpSession(kHost, kPort).Post("/submit", body);
}

std::string SubmitWorkflow(const Workflow &workflow) {
    return Submit(StringifyJSON(Serialize(workflow)));
}

long long Timestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

size_t ParseBlockId(const std::string &container_id) {
    size_t l = container_id.find('_');
    size_t r = container_id.find('_', l + 1);
    return std::stoul(container_id.substr(l + 1, r - l - 1));
}

void ImitateRun(const Workflow &workflow, int runner_delay,
                const std::vector<size_t> &failed_blocks, const RunRequest &request,
                RunResponse &response) {
    fs::path container_path = fs::path(paths::kVarDir) / request.binds[0].outside;
    std::string container_id = container_path.filename().string();
    size_t block_id = ParseBlockId(container_id);
    for (const auto &bind : request.binds) {
        ASSERT_TRUE(fs::exists(fs::path(paths::kVarDir) / bind.outside));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(runner_delay));
    for (const auto &output : workflow.blocks[block_id].outputs) {
        ASSERT_TRUE(!fs::exists(container_path / output));
        std::ofstream((container_path / output).string());
    }
    bool failed =
        std::find(failed_blocks.begin(), failed_blocks.end(), block_id) != failed_blocks.end();
    if (failed) {
        response.error = "Some error";
    } else {
        response.status = {.exited = true, .exit_code = 0};
    }
}

void CheckExecution(const Workflow &workflow, int cnt_clients, int cnt_runners, int exp_runs,
                    int runner_delay, int exp_delay,
                    const std::vector<size_t> &failed_blocks = {}) {
    std::string body = StringifyJSON(Serialize(workflow));
    auto id = HttpSession(kHost, kPort).Post("/submit", body);
    EXPECT_THAT(id, ::testing::MatchesRegex(kUuidRegex));

    static SchemaValidator request_validator(paths::kDataDir + "/schema/run_request.json");
    std::mutex request_validator_mutex;
    std::vector<std::thread> runner_threads(cnt_runners);
    std::vector<WebsocketClientSession> runner_sessions(cnt_runners);
    for (int runner_id = 0; runner_id < cnt_runners; ++runner_id) {
        auto &session = runner_sessions[runner_id];
        runner_threads[runner_id] = std::thread([&] {
            session.Connect(kHost, kPort, "/runner/" + workflow.meta.partition);
            session.OnRead([&](const std::string &message) {
                request_validator_mutex.lock();
                auto document = request_validator.ParseAndValidate(message);
                request_validator_mutex.unlock();
                RunRequest request;
                Deserialize(request, document);
                RunResponse response;
                ImitateRun(workflow, runner_delay, failed_blocks, request, response);
                session.Write(StringifyJSON(Serialize(response)));
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
            session.Connect(kHost, kPort, "/workflow/" + id);
            int cnt_blocks_completed = 0;
            session.OnRead([&](const std::string &message) {
                if (message == states::kComplete) {
                    if (++cnt_clients_completed == cnt_clients) {
                        completed.notify_one();
                    }
                    ASSERT_EQ(cnt_blocks_completed, exp_runs);
                } else {
                    BlockResponse response;
                    Deserialize(response, ParseJSON(message));
                    if (response.state == states::kComplete) {
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
