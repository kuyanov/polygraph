#pragma once

#include <functional>
#include <string>
#include <boost/beast.hpp>

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;

class HttpSession {
public:
    HttpSession(const std::string &host, int port);

    std::string Post(const std::string &target, const std::string &body);

    ~HttpSession();

private:
    asio::io_context ioc_;
    beast::tcp_stream stream_;
};

class WebsocketSession {
public:
    WebsocketSession(asio::io_context &ioc, const std::string &host, int port,
                     const std::string &target);

    void Write(const std::string &message);

    void OnRead(std::function<void(std::string)> handler);

    ~WebsocketSession();

private:
    websocket::stream<ip::tcp::socket> ws_;
    beast::flat_buffer buffer_;
};
