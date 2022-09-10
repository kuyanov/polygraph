#include <filesystem>

#include "run.h"

namespace fs = std::filesystem;

int main() {
    if (!fs::exists(SANDBOX_DIR)) {
        fs::create_directories(SANDBOX_DIR);
    }
    if (!fs::exists(USER_DIR)) {
        fs::create_directories(USER_DIR);
    }
    Run();
}
