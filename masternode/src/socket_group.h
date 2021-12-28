#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include <App.h>

struct UserData {
    std::string group;
};

using WebSocket = uWS::WebSocket<false, true, UserData>;

class SocketGroup {
public:
    void Join(WebSocket *ws);

    void Leave(WebSocket *ws);

    void SendToAll(const std::string &group, std::string_view message, uWS::OpCode op_code);

private:
    std::unordered_map<std::string, std::unordered_set<WebSocket *>> sockets_;
};
