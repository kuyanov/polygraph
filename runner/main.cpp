#include <exception>
#include <iostream>

#include <boost/beast.hpp>

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace http = beast::http;

const char *host = "localhost";
const int kPort = 3000;

int main() {
    try {
        asio::io_context ioc;
        ip::tcp::resolver resolver(ioc);
        websocket::stream<ip::tcp::socket> ws(ioc);

        auto results = resolver.resolve(host, std::to_string(kPort));
        connect(ws.next_layer(), results.begin(), results.end());

        ws.handshake(host, "/submit");
        std::string msg = "{\"blocks\":[],\"connections\":[],\"files\":[]}";
        ws.write(asio::buffer(msg));
        beast::flat_buffer buffer;
        ws.read(buffer);
        std::cout << beast::buffers_to_string(buffer.data()) << std::endl;
        ws.close(websocket::close_code::normal);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
