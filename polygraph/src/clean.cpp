#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <sys/stat.h>

#include "clean.h"
#include "helpers.h"

namespace fs = std::filesystem;

struct FileSize {
    uintmax_t size;

    friend std::ostream &operator<<(std::ostream &out, FileSize fs) {
        int i = 0;
        double mantissa = fs.size;
        for (; mantissa >= 1024.0; mantissa /= 1024.0, ++i) {
        }
        out << std::ceil(mantissa * 10.0) / 10.0 << i["BKMGTPE"];
        if (i > 0) {
            out << "B";
        }
        return out;
    }
};

static uintmax_t AllocatedBytesPosix(const fs::path &p) {
    struct stat st{};
    if (lstat(p.c_str(), &st) != 0) {
        return 0;
    }
    return static_cast<uintmax_t>(st.st_blocks) * 512u;
}

std::pair<uintmax_t, uintmax_t> DirectoryStats(const fs::path &dir) {
    uintmax_t num_items = 0, total_size = 0;
    for (const auto &entry : fs::recursive_directory_iterator(dir)) {
        ++num_items;
        total_size += AllocatedBytesPosix(entry.path());
    }
    return {num_items, total_size};
}

void Clean(const CleanOptions &options) {
    RequireRoot();
    RequireDown();
    fs::path containers_dir = fs::path(VAR_DIR) / "containers";
    auto [num_items, total_size] = DirectoryStats(containers_dir);
    std::cerr << "This operation will remove " << num_items << " items and free "
              << FileSize{total_size} << " of disk space." << std::endl;
    std::cerr << "Continue? [y/N]: ";
    std::string s;
    std::getline(std::cin, s);
    if (!s.empty() && std::tolower(s[0]) == 'y') {
        for (const auto &entry : fs::directory_iterator(containers_dir)) {
            fs::remove_all(entry.path());
        }
        std::cerr << "Done." << std::endl;
    } else {
        std::cerr << "Cancelled." << std::endl;
    }
}
