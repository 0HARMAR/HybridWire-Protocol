#include <memory>
#include <vector>
#include <string>
#include <boost/asio.hpp>
#include "../include/hybridwire_protocol.h"

using namespace boost::asio;
using namespace hybridwire;

class HybridServer {
public:
    HybridServer(io_context& io, unsigned short port)
            : acceptor_(io, ip::tcp::endpoint(ip::tcp::v4(), port)) {
        start_accept();
    }

private:
    void start_accept() {
        auto socket = std::make_shared<ip::tcp::socket>(acceptor_.get_executor());
        acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
            if (!ec) {
                handle_connection(socket);
            }
            start_accept();
        });
    }

    void handle_connection(std::shared_ptr<ip::tcp::socket> socket) {
        auto buffer = std::make_shared<std::string>();

        async_read_until(*socket, dynamic_buffer(*buffer), "\r\n\r\n",
            [this, socket, buffer](boost::system::error_code ec, size_t bytes) {
                if (!ec) {
                    ProtocolHandler handler;
                    auto result = handler.parse(reinterpret_cast<const uint8_t*>(buffer->data()), buffer->size());

                    if (result == ProtocolHandler::ParseResult::HTTP) {
                        send_http_response(socket);
                    } else if (result == ProtocolHandler::ParseResult::BINARY) {
                        handle_binary_protocol(socket, handler);
                    }
                }
            });
    }

    void send_http_response(std::shared_ptr<ip::tcp::socket> socket) {
        std::string response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 13\r\n"
                "\r\n"
                "Hello, Hybrid!";
        async_write(*socket, buffer(response), [socket](auto ec, auto) {});
    }

    void handle_binary_protocol(std::shared_ptr<ip::tcp::socket> socket,
                              const ProtocolHandler& handler) {
        const auto& session_header = handler.get_session_header();
        // 处理二进制协议消息
        // TODO: 实现具体的二进制协议处理逻辑
    }

    ip::tcp::acceptor acceptor_;
};

int main() {
    io_context io;
    HybridServer server(io, 8080);
    io.run();
    return 0;
}