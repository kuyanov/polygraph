#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include "WebSocket.h"

struct UserData {
    std::string group;
};

class ClientsGroup {
    using WebSocket = uWS::WebSocket<true, true, UserData>;

public:
    void join(WebSocket *ws) {
        sockets[ws->getUserData()->group].insert(ws);
    }

    void leave(WebSocket *ws) {
        sockets[ws->getUserData()->group].erase(ws);
    }

    void sendToAll(const std::string &group, std::string_view message, uWS::OpCode opCode) {
        for (WebSocket *ws: sockets[group]) {
            ws->send(message, opCode);
        }
    }

private:
    std::unordered_map<std::string, std::unordered_set<WebSocket *>> sockets;
};
