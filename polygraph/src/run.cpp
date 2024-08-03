#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <unistd.h>

#include "environment.h"
#include "run.h"

namespace fs = std::filesystem;

void Run(const RunOptions &options) {
    setenv("POLYGRAPH_HOST", options.host.c_str(), 1);
    setenv("POLYGRAPH_PORT", std::to_string(options.port).c_str(), 1);
    fs::path exec_path = fs::path(GetExecDir()) / "polygraph-client";
    execl(exec_path.c_str(), "polygraph-client", options.workflow_file.c_str(), nullptr);
    perror("Execution finished with errors");
}
