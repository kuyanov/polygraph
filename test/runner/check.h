#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "constants.h"
#include "json.h"
#include "net.h"
#include "serialization/all.h"
#include "structures/all.h"
#include "uuid.h"

namespace fs = std::filesystem;

const std::string kHost = "0.0.0.0";
const int kPort = 3000;

long long Timestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

fs::path CreateContainer() {
    fs::path container_relpath = fs::path("containers") / GenerateUuid();
    fs::path container_abspath = fs::path(paths::kVarDir) / container_relpath;
    fs::create_directories(container_abspath);
    fs::permissions(container_abspath, fs::perms::all, fs::perm_options::add);
    return container_relpath;
}

void CreateFile(const std::string &relpath, const std::string &content, int other_perms = 7) {
    fs::path abspath = fs::path(paths::kVarDir) / relpath;
    fs::create_directories(abspath.parent_path());
    std::ofstream(abspath.string()) << content;
    fs::permissions(abspath, fs::perms::others_all, fs::perm_options::remove);
    fs::permissions(abspath, static_cast<fs::perms>(other_perms), fs::perm_options::add);
}

std::string ReadFile(const std::string &relpath) {
    fs::path abspath = fs::path(paths::kVarDir) / relpath;
    std::stringstream ss;
    ss << std::ifstream(abspath.string()).rdbuf();
    return ss.str();
}

RunResponse SendRunRequest(const RunRequest &request) {
    static WebsocketServer server(kHost, kPort);
    static SchemaValidator response_validator(paths::kDataDir + "/schema/run_response.json");
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
