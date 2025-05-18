#include <boost/asio.hpp>
#include "hybridwire.hpp"

using namespace boost::asio;
using namespace HybridWire;

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
        auto buffer = std::make_shared<std::vector<uint8_t>>(1024);
        socket->async_read_some(boost::asio::buffer(*buffer),
                                [this, socket, buffer](boost::system::error_code ec, size_t bytes) {
                                    if (!ec) {
                                        ProtocolParser parser;
                                        auto result = parser.parse(buffer->data(), bytes);

                                        if (result == ProtocolParser::ParseResult::HTTP) {
                                            // 处理HTTP请求
                                            send_http_response(socket);
                                        } else if (result == ProtocolParser::ParseResult::CUSTOM) {
                                            // 处理自定义协议
                                            start_session(socket, parser.session_header());
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

    void start_session(std::shared_ptr<ip::tcp::socket> socket,
                       const SessionHeader& header) {
        // 会话管理逻辑
    }

    ip::tcp::acceptor acceptor_;
};

int main() {
    io_context io;
    HybridServer server(io, 8080);
    io.run();
    return 0;
}