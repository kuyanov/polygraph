#pragma once

#include <string>
#include <vector>

#include "block_response.h"
#include "net.h"
#include "workflow.h"

class Client {
public:
    void Run();
    void Stop();

    static Client &Get(const std::string &workflow_file) {
        static Client client(workflow_file);
        return client;
    }

private:
    WebsocketClientSession session_;
    Workflow workflow_;
    std::vector<BlockResponse> blocks_;
    std::vector<size_t> cnt_runs_;

    Client(const std::string &workflow_file);

    void OnMessage(const std::string &message);

    void PrintWarnings();
    void PrintBlocks();
    void PrintErrors();
};
