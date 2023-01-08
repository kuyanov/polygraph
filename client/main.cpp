#include <csignal>
#include <memory>

#include "client.h"
#include "options.h"

std::unique_ptr<Client> client;

int main(int argc, char **argv) {
    Options::Get().Init(argc, argv);
    client = std::make_unique<Client>();
    signal(SIGINT, [](int) { client->Stop(); });
    client->Run();
}
