#include <boost/beast.hpp>
#include <exception>
#include <iostream>

using namespace boost::asio;
using namespace boost::beast;
using namespace std;

const char *HOST = "localhost";
const int PORT = 3000;

int main() {
    try {
        io_context ioc;
        ip::tcp::resolver resolver(ioc);
        websocket::stream<ip::tcp::socket> ws(ioc);
        auto results = resolver.resolve(HOST, to_string(PORT));
        connect(ws.next_layer(), results.begin(), results.end());

        ws.handshake(HOST, "/test");
        string msg = "Hello, world!";
        ws.write(buffer(msg));
        flat_buffer buffer;
        ws.read(buffer);
        cout << buffers_to_string(buffer.data()) << endl;
        ws.close(websocket::close_code::normal);
    } catch (const exception &e) {
        cerr << e.what() << endl;
        return 1;
    }
}
