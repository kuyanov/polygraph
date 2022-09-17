#pragma once

#include <string>
#include <vector>

#include "graph.h"
#include "net.h"
#include "result.h"

class Client {
public:
    explicit Client(const std::string &graph_path);

    void Run();
    void Stop();

private:
    WebsocketClientSession session_;
    Graph graph_;
    std::vector<BlockResponse> blocks_;
    bool blocks_printed_ = false;

    void HandleMessage(const std::string &message);

    void PrintBlocks();
    void PrintErrors();
};
