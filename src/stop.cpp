#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>

#include "helpers.h"
#include "runner_stop.h"
#include "stop.h"

namespace fs = std::filesystem;

void Stop(const StopOptions &options) {
    RequireRoot();
    RequireUp();
    RunnerStop({});
    fs::path pid_path = fs::path(RUN_DIR) / "scheduler.pid";
    std::ifstream pid_file(pid_path.string());
    pid_t scheduler_pid;
    pid_file >> scheduler_pid;
    if (kill(scheduler_pid, SIGTERM)) {
        perror("Failed to stop scheduler");
        exit(EXIT_FAILURE);
    }
    fs::remove(pid_path);
}
