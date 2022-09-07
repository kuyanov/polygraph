#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "config.h"
#include "json.h"
#include "net.h"
#include "operations.h"
#include "result.h"
#include "task.h"
#include "uuid.h"

namespace fs = std::filesystem;

const std::string kTestContainer = GenerateUuid();

static WebsocketServer server(Config::Get().scheduler_host, Config::Get().scheduler_port);
static SchemaValidator response_validator("run_response.json");

long long Timestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

void InitContainer() {
    fs::path container_path = fs::path(SANDBOX_DIR) / kTestContainer;
    if (fs::exists(container_path)) {
        fs::remove_all(container_path);
    }
    fs::create_directories(container_path);
}

void AddFile(const std::string &filename, const std::string &content) {
    fs::path filepath = fs::path(SANDBOX_DIR) / kTestContainer / filename;
    std::ofstream(filepath) << content;
}

std::string ReadFile(const std::string &filename) {
    fs::path filepath = fs::path(SANDBOX_DIR) / kTestContainer / filename;
    std::stringstream ss;
    ss << std::ifstream(filepath).rdbuf();
    return ss.str();
}

RunResponse SendTask(const Task &task) {
    RunRequest run_request = {.container = kTestContainer, .task = task};
    auto session = server.Accept();
    session.Write(StringifyJSON(Dump(run_request)));
    RunResponse run_response;
    Load(run_response, response_validator.ParseAndValidate(session.Read()));
    return run_response;
}

void CheckDuration(long long duration, long long expected) {
    ASSERT_GE(duration, expected);
    ASSERT_LT(duration, expected + 100);
}

void CheckExitedNormally(const RunResponse &run_response) {
    ASSERT_TRUE(run_response.result.has_value());
    ASSERT_TRUE(run_response.result->exited);
    ASSERT_EQ(run_response.result->exit_code, 0);
}
