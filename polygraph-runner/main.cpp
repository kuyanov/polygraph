#include <string>

#include "config.h"
#include "logger.h"
#include "run.h"

int main() {
    Logger::Get().SetName(std::string("runner ") + std::to_string(RunnerConfig::Get().id));
    Run();
}
