#include "net.h"

HttpSession::HttpSession(const std::string &host, int port) : host_(host), stream_(ioc_) {
    ip::tcp::resolver resolver(ioc_);
    auto results = resolver.resolve(host, std::to_string(port));
    stream_.connect(results);
}

std::string HttpSession::Post(const std::string &target, const std::string &body) {
    http::request<http::string_body> req{http::verb::post, target, 11};
    req.set(http::field::host, host_);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.body() = body;
    req.prepare_payload();
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

WebsocketClientSession::WebsocketClientSession() : ws_(ioc_) {
}

void WebsocketClientSession::Connect(const std::string &host, int port, const std::string &target) {
    ip::tcp::resolver resolver(ioc_);
    auto results = resolver.resolve(host, std::to_string(port));
    connect(ws_.next_layer(), results.begin(), results.end());
    ws_.handshake(host, target);
}

void WebsocketClientSession::Write(const std::string &message) {
    ws_.write(asio::buffer(message));
}

void WebsocketClientSession::OnRead(std::function<void(std::string)> handler) {
    ws_.template async_read(buffer_, [this, handler = std::move(handler)](
                                         const beast::error_code &ec, size_t bytes_written) {
        if (ec) {
            throw beast::system_error(ec);
        }
        std::string message = beast::buffers_to_string(buffer_.data());
        handler(message);
        buffer_.clear();
        OnRead(handler);
    });
}

void WebsocketClientSession::Run() {
    ioc_.run();
}

void WebsocketClientSession::Stop() {
    ioc_.stop();
}

WebsocketClientSession::~WebsocketClientSession() {
    beast::error_code ec;
    ws_.close(websocket::close_code::normal, ec);
}

WebsocketServerSession::WebsocketServerSession(websocket::stream<beast::tcp_stream> ws)
    : ws_(std::move(ws)) {
}

std::string WebsocketServerSession::Read() {
    beast::flat_buffer buffer;
    ws_.read(buffer);
    return beast::buffers_to_string(buffer.data());
}

void WebsocketServerSession::Write(const std::string &message) {
    ws_.write(boost::asio::buffer(message));
}

WebsocketServerSession::~WebsocketServerSession() {
    beast::error_code ec;
    ws_.close(websocket::close_code::normal, ec);
}

WebsocketServer::WebsocketServer(const std::string &host, int port)
    : endpoint_(asio::ip::make_address(host), static_cast<unsigned short>(port)) {
}

WebsocketServerSession WebsocketServer::Accept() {
    ip::tcp::acceptor acceptor(ioc_, endpoint_);
    ip::tcp::socket socket(ioc_);
    acceptor.accept(socket);
    websocket::stream<beast::tcp_stream> ws(std::move(socket));
    ws.accept();
    return WebsocketServerSession(std::move(ws));
}
