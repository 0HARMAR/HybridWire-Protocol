#include <iostream>
#include <boost/asio.hpp>
#include "../include/hwp.hpp"

using namespace boost::asio;

namespace hwp {
namespace client {

class Client::Impl {
public:
    Impl(const std::string& host, unsigned short port)
        : io_(), socket_(io_), endpoint_(ip::address::from_string(host), port) {}

    // 连接到服务器
    bool connect() {
        try {
            socket_.connect(endpoint_);
            return true;
        } catch (std::exception& e) {
            std::cerr << "连接错误: " << e.what() << std::endl;
            return false;
        }
    }

    // 发送HTTP模式的请求
    std::string sendHttpRequest(const std::string& http_request) {
        try {
            // 创建基础协议头
            Message msg;
            msg.base_header.magic[0] = 'H';
            msg.base_header.magic[1] = 'W';
            msg.base_header.magic[2] = 'P';
            msg.base_header.magic[3] = '\0';
            msg.base_header.version = PROTOCOL_VERSION;
            msg.base_header.flags = static_cast<uint8_t>(Flags::HTTP_MODE);
            msg.base_header.head_len = htons(sizeof(BaseHeader));
            
            // 发送头部
            write(socket_, buffer(&msg.base_header, sizeof(BaseHeader)));
            
            // 发送HTTP请求
            write(socket_, buffer(http_request));

            // 读取响应
            std::vector<char> response(1024);
            size_t len = socket_.read_some(buffer(response));
            return std::string(response.begin(), response.begin() + len);
        } catch (std::exception& e) {
            std::cerr << "请求错误: " << e.what() << std::endl;
            return "";
        }
    }

    // 发送二进制模式的消息
    bool sendBinaryMessage(const std::vector<uint8_t>& payload, MessageType type) {
        try {
            // 创建完整消息
            Message msg = ProtocolHandler::create_message(
                type,
                0, // 新会话时session_id为0
                payload,
                static_cast<uint8_t>(Flags::BINARY_MODE)
            );

            // 序列化并发送消息
            std::vector<uint8_t> serialized = ProtocolHandler::serialize_message(msg);
            write(socket_, buffer(serialized));
            return true;
        } catch (std::exception& e) {
            std::cerr << "发送错误: " << e.what() << std::endl;
            return false;
        }
    }

    // 关闭连接
    void close() {
        if (socket_.is_open()) {
            socket_.close();
        }
    }

private:
    io_context io_;
    ip::tcp::socket socket_;
    ip::tcp::endpoint endpoint_;
};

// Client class implementation
Client::Client(const std::string& host, unsigned short port)
    : impl_(std::make_unique<Impl>(host, port)) {}

Client::~Client() = default;

bool Client::connect() {
    return impl_->connect();
}

std::string Client::sendHttpRequest(const std::string& http_request) {
    return impl_->sendHttpRequest(http_request);
}

bool Client::sendBinaryMessage(const std::vector<uint8_t>& payload, MessageType type) {
    return impl_->sendBinaryMessage(payload, type);
}

void Client::close() {
    impl_->close();
}

} // namespace client
} // namespace hwp 