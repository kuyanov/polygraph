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
    void Join(WebSocket *ws) {
        sockets_[ws->getUserData()->group].insert(ws);
    }

    void Leave(WebSocket *ws) {
        sockets_[ws->getUserData()->group].erase(ws);
    }

    void SendToAll(const std::string &group, std::string_view message, uWS::OpCode op_code) {
        for (WebSocket *ws: sockets_[group]) {
            ws->send(message, op_code);
        }
    }

private:
    std::unordered_map<std::string, std::unordered_set<WebSocket *>> sockets_;
};
