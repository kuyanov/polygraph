#include <filesystem>

#include "run.h"

namespace fs = std::filesystem;

int main() {
    if (!fs::exists(SANDBOX_DIR)) {
        fs::create_directories(SANDBOX_DIR);
    }
    Run();
}
