#pragma once

#include <string>
#include <vector>

#include "net.h"
#include "structures/all.h"

class Client {
public:
    Client(const std::string &workflow_path);

    void Run();
    void Stop();

private:
    WebsocketClientSession session_;
    Workflow workflow_;
    std::vector<BlockResponse> blocks_;
    std::vector<size_t> cnt_runs_;

    void OnMessage(const std::string &message);

    void PrintWarnings();
    void PrintBlocks();
    void PrintErrors();
};
