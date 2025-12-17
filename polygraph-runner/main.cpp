#include <string>

#include "environment.h"
#include "logger.h"
#include "run.h"

int main() {
    std::string id = GetEnvOr("RUNNER_ID", "0");
    std::string partition = GetEnvOr("RUNNER_PARTITION", "all");
    std::string name = std::string("runner ") + id;
    Logger::Get().SetName(name);
    Run(id, partition);
}
