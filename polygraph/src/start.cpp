#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>

#include "helpers.h"
#include "start.h"

namespace fs = std::filesystem;

void Start(const StartOptions &options) {
    RequireRoot();
    CreateDirs();
    fs::path pid_path = fs::path(RUN_DIR) / "scheduler.pid";
    if (fs::exists(pid_path)) {
        std::cerr << "File " << pid_path << " already exists" << std::endl;
        exit(EXIT_FAILURE);
    }
    Daemonize();
    std::ofstream pid_file(pid_path.string());
    pid_file << getpid() << std::endl;
    fs::path exec_path = fs::path(EXEC_DIR) / "polygraph-scheduler";
    execl(exec_path.c_str(), "polygraph-scheduler", nullptr);
    fs::remove(pid_path);
    perror("Failed to start scheduler");
    exit(EXIT_FAILURE);
}
