#include <iostream>

#include "run.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " PATH, where PATH is the absolute path to graph json"
                  << std::endl;
        return 1;
    }
    Run(argv[1]);
}
