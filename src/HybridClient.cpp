#include <iostream>
#include <boost/asio.hpp>
#include "../include/hybridwire.hpp"

using namespace boost::asio;
using namespace HybridWire;

class HybridClient {
public:
    HybridClient(const std::string& host, unsigned short port)
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
            // 创建协议头
            ProtocolHeader header;
            memcpy(header.magic, "HWP", 3);
            header.magic[3] = '\0';
            header.version = 0x01;
            header.flags = 0x01;  // HTTP模式
            header.head_len = htonl(sizeof(ProtocolHeader));

            // 发送头部
            write(socket_, buffer(&header, sizeof(header)));
            
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