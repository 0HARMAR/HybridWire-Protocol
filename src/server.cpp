#include <memory>
#include <vector>
#include <string>
#include <boost/asio.hpp>
#include "../include/hwp.hpp"

using namespace boost::asio;

namespace hwp {
namespace server {

class Server::Impl {
public:
    Impl(io_context& io, unsigned short port)
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
            [this, socket, buffer](boost::system::error_code ec, size_t /*bytes*/) {
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
        async_write(*socket, buffer(response), [socket](boost::system::error_code /*ec*/, size_t /*bytes*/) {});
    }

    void handle_binary_protocol(std::shared_ptr<ip::tcp::socket> /*socket*/,
                              const ProtocolHandler& /*handler*/) {
        // TODO: 实现具体的二进制协议处理逻辑
    }

    ip::tcp::acceptor acceptor_;
};

// Server class implementation
Server::Server(io_context& io, unsigned short port)
    : impl_(std::make_unique<Impl>(io, port)) {}

Server::~Server() = default;

} // namespace server
} // namespace hwp
