#pragma once

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <unistd.h>

namespace fs = std::filesystem;

inline void RequireRoot() {
    if (geteuid() != 0) {
        std::cerr << "Error: this command requires root privileges." << std::endl;
        exit(EXIT_FAILURE);
    }
}

inline void RequireUp() {
    fs::path pid_path = fs::path(RUN_DIR) / "scheduler.pid";
    if (!fs::exists(pid_path)) {
        std::cerr << "Error: polygraph is not running (file " << pid_path << " does not exist)."
                  << std::endl;
        std::cerr << "To execute this command, start polygraph first." << std::endl;
        exit(EXIT_FAILURE);
    }
}

inline void RequireDown() {
    fs::path pid_path = fs::path(RUN_DIR) / "scheduler.pid";
    if (fs::exists(pid_path)) {
        std::cerr << "Error: polygraph is running (file " << pid_path << " exists)." << std::endl;
        std::cerr << "To execute this command, stop polygraph first." << std::endl;
        exit(EXIT_FAILURE);
    }
}

inline void CreateDirs() {
    if (geteuid() == 0) {
        fs::create_directories(fs::path(CONTAINERS_DIR));
        fs::create_directories(fs::path(RUN_DIR));
    }
}

inline void Daemonize() {
    fs::path log_path = LOG_PATH;
    FILE *fs_null = fopen("/dev/null", "r+");
    FILE *fs_log = fopen(log_path.c_str(), "a");
    if (!fs_null) {
        perror("Failed to open /dev/null");
        exit(EXIT_FAILURE);
    }
    if (!fs_log) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }
    dup2(fileno(fs_null), STDIN_FILENO);
    dup2(fileno(fs_null), STDOUT_FILENO);
    dup2(fileno(fs_log), STDERR_FILENO);
    fclose(fs_null);
    fclose(fs_log);
    if (daemon(1, 1)) {
        perror("Failed to daemonize");
        exit(EXIT_FAILURE);
    }
}
