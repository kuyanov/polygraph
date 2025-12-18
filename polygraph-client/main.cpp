#include <csignal>
#include <cstdlib>
#include <functional>
#include <iostream>

#include "client.h"

std::function<void(int)> interrupt_handler;

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Wrong number of arguments." << std::endl;
        exit(EXIT_FAILURE);
    }
    Client client(argv[1]);
    interrupt_handler = [&client](int signum) { client.Stop(); };
    signal(SIGINT, [](int signum) { interrupt_handler(signum); });
    client.Run();
}
