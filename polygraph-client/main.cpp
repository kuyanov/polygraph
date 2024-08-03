#include <csignal>
#include <cstdlib>
#include <functional>
#include <iostream>

#include "client.h"

std::function<void()> interrupt_handler;

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Wrong number of arguments." << std::endl;
        exit(EXIT_FAILURE);
    }
    Client client(argv[1]);
    interrupt_handler = [&client] { client.Stop(); };
    signal(SIGINT, [](int) { interrupt_handler(); });
    client.Run();
}
