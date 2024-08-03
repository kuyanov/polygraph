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
    std::string host_;
    asio::io_context ioc_;
    beast::tcp_stream stream_;
};

class WebsocketClientSession {
public:
    WebsocketClientSession();

    void Connect(const std::string &host, int port, const std::string &target);

    void Write(const std::string &message);
    void OnRead(std::function<void(std::string)> handler);

    void Run();
    void Stop();

    ~WebsocketClientSession();

private:
    asio::io_context ioc_;
    websocket::stream<ip::tcp::socket> ws_;
    beast::flat_buffer buffer_;
};

class WebsocketServerSession {
public:
    WebsocketServerSession(websocket::stream<beast::tcp_stream> ws);

    std::string Read();
    void Write(const std::string &message);

    ~WebsocketServerSession();

private:
    websocket::stream<beast::tcp_stream> ws_;
};

class WebsocketServer {
public:
    WebsocketServer(const std::string &host, int port);

    WebsocketServerSession Accept();

private:
    asio::io_context ioc_;
    ip::tcp::endpoint endpoint_;
};
