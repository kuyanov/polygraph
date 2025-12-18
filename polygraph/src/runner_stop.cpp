#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "helpers.h"
#include "runner_stop.h"

namespace fs = std::filesystem;

std::vector<int> GetAllIds() {
    std::vector<int> ids;
    for (const auto &entry : fs::directory_iterator(fs::path(RUN_DIR))) {
        auto filename = entry.path().filename().string();
        if (filename.starts_with("runner_")) {
            size_t l = 7;
            size_t r = filename.find('.');
            ids.push_back(stoi(filename.substr(l, r - l)));
        }
    }
    return ids;
}

void RunnerStop(const RunnerStopOptions &options) {
    RequireRoot();
    RequireUp();
    auto ids = !options.ids.empty() ? options.ids : GetAllIds();
    for (int runner_id : ids) {
        fs::path runner_pid_path =
            fs::path(RUN_DIR) / ("runner_" + std::to_string(runner_id) + ".pid");
        if (!fs::exists(runner_pid_path)) {
            std::cerr << "Failed to stop runner " << runner_id << ": file " << runner_pid_path
                      << " does not exist" << std::endl;
            exit(EXIT_FAILURE);
        }
        std::ifstream runner_pid_file(runner_pid_path.string());
        pid_t runner_pid;
        runner_pid_file >> runner_pid;
        if (kill(runner_pid, SIGTERM)) {
            perror("Failed to stop runner");
            exit(EXIT_FAILURE);
        }
        fs::remove(runner_pid_path);
    }
}
