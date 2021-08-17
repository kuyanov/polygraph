#include <iostream>
#include "App.h"

using namespace uWS;
using namespace std;

const int PORT = 3000;

struct UserData {
};

int main() {
    App().ws<UserData>("/*", {
        .message = [](auto *ws, string_view message, OpCode opCode) {
            ws->send(message, opCode);
        }
    }).listen(PORT, [](auto *listen_socket) {
        if (listen_socket) {
            cout << "Listening on port " << PORT << endl;
        } else {
            cout << "Failed to listen on port " << PORT << endl;
        }
    }).run();
}
