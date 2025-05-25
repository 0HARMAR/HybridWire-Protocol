#include <iostream>
#include <thread>
#include <chrono>
#include "../include/hwp.hpp"

using namespace std::chrono_literals;

int main() {
    try {
        // 创建IO上下文
        boost::asio::io_context io;
        
        // 创建服务器 (监听在8080端口)
        hwp::server::Server server(io, 8080);
        
        // 在单独的线程中运行服务器
        std::thread server_thread([&io]() {
            std::cout << "服务器启动在 8080 端口...\n";
            io.run();
        });
        
        // 等待服务器启动
        std::this_thread::sleep_for(1s);
        
        // 创建客户端
        hwp::client::Client client("127.0.0.1", 8080);
        
        // 连接到服务器
        if (client.connect()) {
            std::cout << "客户端已连接到服务器\n";
            
            // 发送HTTP请求
            std::string http_request = 
                "GET / HTTP/1.1\r\n"
                "Host: localhost:8080\r\n"
                "Connection: close\r\n"
                "\r\n";
            
            std::cout << "发送HTTP请求...\n";
            std::string response = client.sendHttpRequest(http_request);
            
            std::cout << "收到响应:\n" << response << std::endl;
            
            // 关闭客户端连接
            client.close();
        } else {
            std::cerr << "无法连接到服务器\n";
        }
        
        // 停止服务器
        io.stop();
        server_thread.join();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
} 