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
#include "run_request.h"
#include "run_response.h"

namespace fs = std::filesystem;

const std::string kContainerName = "test";

static WebsocketServer server(Config::Get().scheduler_host, Config::Get().scheduler_port);
static SchemaValidator response_validator("run_response.json");

long long Timestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

void InitContainer() {
    fs::path container_path = fs::path(SANDBOX_DIR) / kContainerName;
    if (fs::exists(container_path)) {
        fs::remove_all(container_path);
    }
    fs::create_directories(container_path);
}

void AddFile(const std::string &filename, const std::string &content) {
    fs::path filepath = fs::path(SANDBOX_DIR) / kContainerName / filename;
    std::ofstream(filepath) << content;
}

std::string ReadFile(const std::string &filename) {
    fs::path filepath = fs::path(SANDBOX_DIR) / kContainerName / filename;
    std::stringstream ss;
    ss << std::ifstream(filepath).rdbuf();
    return ss.str();
}

void InitUserDir() {
    if (fs::exists(USER_DIR)) {
        fs::remove_all(USER_DIR);
    }
    fs::create_directories(USER_DIR);
}

void AddUserFile(const std::string &filename, const std::string &content, bool executable = false) {
    fs::path filepath = fs::path(USER_DIR) / filename;
    std::ofstream(filepath) << content;
    if (executable) {
        fs::permissions(filepath, fs::perms::others_exec, fs::perm_options::add);
    }
}

std::string ReadUserFile(const std::string &filename) {
    fs::path filepath = fs::path(USER_DIR) / filename;
    std::stringstream ss;
    ss << std::ifstream(filepath).rdbuf();
    return ss.str();
}

RunResponse SendTasks(const std::vector<Task> &tasks) {
    RunRequest run_request = {.container = kContainerName, .tasks = tasks};
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

void CheckAllExitedNormally(const RunResponse &run_response) {
    ASSERT_TRUE(!run_response.has_error);
    for (const auto &result : run_response.results) {
        ASSERT_TRUE(result.exited);
        ASSERT_EQ(result.exit_code, 0);
    }
}
