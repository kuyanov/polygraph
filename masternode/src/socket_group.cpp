#include "socket_group.h"

void SocketGroup::Join(WebSocket *ws) {
    sockets_[ws->getUserData()->group].insert(ws);
}

void SocketGroup::Leave(WebSocket *ws) {
    sockets_[ws->getUserData()->group].erase(ws);
}

void SocketGroup::SendToAll(const std::string &group, std::string_view message,
                            uWS::OpCode op_code) {
    for (WebSocket *ws: sockets_[group]) {
        ws->send(message, op_code);
    }
}
