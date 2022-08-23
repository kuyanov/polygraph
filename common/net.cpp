#include "net.h"

HttpSession::HttpSession(const std::string &host, int port) : stream_(ioc_) {
    ip::tcp::resolver resolver(ioc_);
    auto results = resolver.resolve(host, std::to_string(port));
    stream_.connect(results);
}

std::string HttpSession::Post(const std::string &target, const std::string &body) {
    http::request<http::string_body> req{http::verb::post, target, 10};
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::content_length, std::to_string(body.size()));
    req.body() = body;
    http::write(stream_, req);
    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(stream_, buffer, res);
    return res.body();
}

HttpSession::~HttpSession() {
    beast::error_code ec;
    stream_.socket().shutdown(ip::tcp::socket::shutdown_both, ec);
}

WebsocketSession::WebsocketSession(asio::io_context &ioc, const std::string &host, int port,
                                   const std::string &target)
    : ws_(ioc) {
    ip::tcp::resolver resolver(ioc);
    auto results = resolver.resolve(host, std::to_string(port));
    connect(ws_.next_layer(), results.begin(), results.end());
    ws_.handshake(host, target);
}

void WebsocketSession::Write(const std::string &message) {
    ws_.write(asio::buffer(message));
}

void WebsocketSession::OnRead(std::function<void(std::string)> handler) {
    ws_.template async_read(buffer_, [this, handler = std::move(handler)](
                                         const beast::error_code &ec, size_t bytes_written) {
        std::string message = beast::buffers_to_string(buffer_.data());
        handler(message);
        buffer_.clear();
        OnRead(handler);
    });
}

WebsocketSession::~WebsocketSession() {
    beast::error_code ec;
    ws_.close(websocket::close_code::normal, ec);
}