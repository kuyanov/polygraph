#include <cstdlib>
#include <iostream>
#include <string>

#include "logger.h"
#include "run.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage:  prunner ID PARTITION" << std::endl;
        exit(EXIT_FAILURE);
    }
    Logger::Get().SetName(std::string("runner ") + argv[1]);
    Run(argv[2]);
}
