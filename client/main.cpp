#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "client.h"

std::unique_ptr<Client> client;

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage:  pclient WORKFLOW_PATH" << std::endl;
        exit(EXIT_FAILURE);
    }
    client = std::make_unique<Client>(argv[1]);
    signal(SIGINT, [](int) { client->Stop(); });
    client->Run();
}
