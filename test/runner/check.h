#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "config.h"
#include "json.h"
#include "net.h"
#include "run_request.h"
#include "run_response.h"
#include "uuid.h"

namespace fs = std::filesystem;

long long Timestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

std::string CreateContainer() {
    fs::path container_path = fs::path(CONTAINERS_DIR) / GenerateUuid();
    fs::create_directories(container_path);
    fs::permissions(container_path, fs::perms::all, fs::perm_options::add);
    return container_path.string();
}

void CreateFile(const std::string &abspath, const std::string &content, int other_perms = 7) {
    fs::create_directories(fs::path(abspath).parent_path());
    std::ofstream(abspath) << content;
    fs::permissions(fs::path(abspath), fs::perms::others_all, fs::perm_options::remove);
    fs::permissions(fs::path(abspath), static_cast<fs::perms>(other_perms), fs::perm_options::add);
}

std::string ReadFile(const std::string &abspath) {
    std::stringstream ss;
    ss << std::ifstream(abspath).rdbuf();
    return ss.str();
}

RunResponse SendRunRequest(const RunRequest &request) {
    static WebsocketServer server("0.0.0.0", Config::Get().port);
    static SchemaValidator response_validator(SCHEMA_DIR "/run_response.json");
    auto session = server.Accept();
    session.Write(StringifyJSON(Serialize(request)));
    RunResponse response;
    Deserialize(response, response_validator.ParseAndValidate(session.Read()));
    return response;
}

void CheckDuration(long long duration, long long expected) {
    ASSERT_GE(duration, expected);
    ASSERT_LT(duration, expected + 100);
}

void CheckExitedNormally(const RunResponse &response) {
    ASSERT_TRUE(response.status.has_value());
    ASSERT_TRUE(response.status->exited);
    ASSERT_EQ(response.status->exit_code, 0);
}
