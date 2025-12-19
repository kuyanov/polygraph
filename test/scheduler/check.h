#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "block_response.h"
#include "config.h"
#include "definitions.h"
#include "json.h"
#include "net.h"
#include "run_request.h"
#include "run_response.h"
#include "submit_response.h"
#include "workflow.h"

namespace fs = std::filesystem;

SubmitResponse Submit(const std::string &body) {
    std::string submit_response_text =
        HttpSession(Config::Get().host, Config::Get().port).Post("/submit", body);
    if (submit_response_text.empty()) {
        throw std::runtime_error("submit response empty");
    }
    SubmitResponse submit_response;
    Deserialize(submit_response, ParseJSON(submit_response_text));
    return submit_response;
}

SubmitResponse SubmitWorkflow(const Workflow &workflow) {
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
    fs::path container_path = fs::path(request.binds[0].outside);
    std::string container_id = container_path.filename().string();
    size_t block_id = ParseBlockId(container_id);
    for (const auto &bind : request.binds) {
        ASSERT_TRUE(fs::exists(bind.outside));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(runner_delay));
    for (const auto &output : workflow.blocks[block_id].outputs) {
        ASSERT_TRUE(!fs::exists(container_path / output.path));
        std::ofstream((container_path / output.path).string());
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
    auto submit_response = SubmitWorkflow(workflow);
    EXPECT_EQ(submit_response.status, SUBMIT_ACCEPTED);
    std::string workflow_id = submit_response.data;

    static SchemaValidator request_validator(SCHEMA_DIR "/run_request.json");
    std::mutex request_validator_mutex;
    std::vector<std::thread> runner_threads(cnt_runners);
    std::vector<WebsocketClientSession> runner_sessions(cnt_runners);
    for (int runner_id = 0; runner_id < cnt_runners; ++runner_id) {
        auto &session = runner_sessions[runner_id];
        runner_threads[runner_id] = std::thread([&, runner_id] {
            session.Connect(Config::Get().host, Config::Get().port,
                            "/runner/" + workflow.meta.partition + "/" + std::to_string(runner_id));
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
            session.Connect(Config::Get().host, Config::Get().port, "/workflow/" + workflow_id);
            int cnt_blocks_completed = 0;
            session.OnRead([&](const std::string &message) {
                if (message.starts_with(WORKFLOW_SIGNAL)) {
                    if (message.substr(strlen(WORKFLOW_SIGNAL) + 1) == FINISHED_STATE) {
                        if (++cnt_clients_completed == cnt_clients) {
                            completed.notify_one();
                        }
                        ASSERT_EQ(cnt_blocks_completed, exp_runs);
                    }
                } else if (message.starts_with(BLOCK_SIGNAL)) {
                    std::string response_text = message.substr(strlen(BLOCK_SIGNAL) + 1);
                    BlockResponse response;
                    Deserialize(response, ParseJSON(response_text));
                    if (response.state == FINISHED_STATE) {
                        ++cnt_blocks_completed;
                    }
                }
            });
            if (++cnt_clients_connected == cnt_clients) {
                session.Write(RUN_SIGNAL);
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
