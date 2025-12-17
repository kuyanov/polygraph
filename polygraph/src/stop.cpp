#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>

#include "helpers.h"
#include "runner_stop.h"
#include "stop.h"

namespace fs = std::filesystem;

void Stop(const StopOptions &options) {
    RequireRoot();
    RunnerStop({});
    fs::path scheduler_pid_path = fs::path(RUN_DIR) / "scheduler.pid";
    if (!fs::exists(scheduler_pid_path)) {
        std::cerr << "Failed to stop scheduler: file " << scheduler_pid_path << " does not exist"
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    std::ifstream scheduler_pid_file(scheduler_pid_path.string());
    pid_t scheduler_pid;
    scheduler_pid_file >> scheduler_pid;
    if (kill(scheduler_pid, SIGTERM)) {
        perror("Failed to stop scheduler");
        exit(EXIT_FAILURE);
    }
    fs::remove(scheduler_pid_path);
}
