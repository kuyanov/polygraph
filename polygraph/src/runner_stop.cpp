#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <vector>

#include "common.h"
#include "runner_stop.h"

namespace fs = std::filesystem;

std::vector<int> GetAllIds() {
    std::vector<int> ids;
    for (const auto &entry : fs::directory_iterator(fs::path(GetRunDir()))) {
        auto filename = entry.path().filename().string();
        if (filename.starts_with("runner")) {
            size_t l = 6;
            size_t r = filename.find('.');
            ids.push_back(atoi(filename.substr(l, r - l).c_str()));
        }
    }
    return ids;
}

void RunnerStop(const RunnerStopOptions &options) {
    RequireRoot();
    auto ids = !options.ids.empty() ? options.ids : GetAllIds();
    for (int runner_id : ids) {
        fs::path runner_pid_path = fs::path(GetRunDir()) /
                                   ("runner" + std::to_string(runner_id) + ".pid");
        if (!fs::exists(runner_pid_path)) {
            std::cerr << "Failed to stop runner " << runner_id << ": file "
                      << runner_pid_path.filename() << " does not exist" << std::endl;
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
