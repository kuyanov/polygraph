#include <boost/beast.hpp>
#include <exception>
#include <iostream>

const char *host = "localhost";
const int kPort = 3000;

int main() {
    try {
        boost::asio::io_context ioc;
        boost::asio::ip::tcp::resolver resolver(ioc);
        boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws(ioc);

        auto results = resolver.resolve(host, std::to_string(kPort));
        connect(ws.next_layer(), results.begin(), results.end());

        ws.handshake(host, "/test");
        std::string msg = "Hello, world!";
        ws.write(boost::asio::buffer(msg));
        boost::beast::flat_buffer buffer;
        ws.read(buffer);
        std::cout << boost::beast::buffers_to_string(buffer.data()) << std::endl;
        ws.close(boost::beast::websocket::close_code::normal);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
