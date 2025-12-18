#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <unistd.h>

#include "helpers.h"
#include "run.h"

namespace fs = std::filesystem;

void Run(const RunOptions &options) {
    RequireUp();
    fs::path exec_path = fs::path(EXEC_DIR) / "polygraph-client";
    execl(exec_path.c_str(), "polygraph-client", options.workflow_file.c_str(), nullptr);
    perror("Failed to run");
}
