#include <iostream>
#include <memory>
#include <signal.h>

#include "client.h"

std::unique_ptr<Client> client;

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " PATH" << std::endl;
        return 1;
    }
    client = std::make_unique<Client>(argv[1]);
    signal(SIGINT, [](int) { client->Stop(); });
    client->Run();
}
