#pragma once

#include <string>
#include <vector>

#include "net.h"
#include "structures.h"

class Client {
public:
    explicit Client(const std::string &workflow_path);

    void Run();
    void Stop();

private:
    WebsocketClientSession session_;
    Workflow workflow_;
    std::vector<BlockResponse> blocks_;

    void OnMessage(const std::string &message);

    void PrintBlocks();
    void PrintErrors();
};
