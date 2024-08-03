#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>

#include "common.h"
#include "start.h"

namespace fs = std::filesystem;

void Start(const StartOptions &options) {
    RequireRoot();
    CreateDirs();
    fs::path pid_path = fs::path(GetRunDir()) / "scheduler.pid";
    if (fs::exists(pid_path)) {
        std::cerr << "File scheduler.pid already exists" << std::endl;
        exit(EXIT_FAILURE);
    }
    Daemonize();
    std::ofstream pid_file(pid_path.string());
    pid_file << getpid() << std::endl;
    setenv("POLYGRAPH_PORT", std::to_string(options.port).c_str(), 1);
    setenv("POLYGRAPH_SCHEDULER_MPLEN", std::to_string(options.max_payload_length).c_str(), 1);
    setenv("POLYGRAPH_SCHEDULER_IDLE_TIMEOUT_S", std::to_string(options.idle_timeout).c_str(), 1);
    fs::path exec_path = fs::path(GetExecDir()) / "polygraph-scheduler";
    execl(exec_path.c_str(), "polygraph-scheduler", nullptr);
    fs::remove(pid_path);
    perror("Failed to start scheduler");
    exit(EXIT_FAILURE);
}
