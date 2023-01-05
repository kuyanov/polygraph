#pragma once

#include <string>
#include <vector>
#include <rapidjson/document.h>

#include "net.h"
#include "structures/all.h"

class Client {
public:
    explicit Client(const rapidjson::Document &document, const std::string &scheduler_host,
                    int scheduler_port);

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
