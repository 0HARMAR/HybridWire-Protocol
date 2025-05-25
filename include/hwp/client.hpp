#ifndef HWP_CLIENT_HPP
#define HWP_CLIENT_HPP

#include <memory>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <cstdint>

namespace hwp {

// 前向声明
enum class MessageType : uint8_t;

namespace client {

// Client-side functionality will be implemented here
class Client {
public:
    Client(const std::string& host, unsigned short port);
    ~Client();
    
    // 禁用拷贝
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    
    // 公共接口
    bool connect();
    std::string sendHttpRequest(const std::string& http_request);
    bool sendBinaryMessage(const std::vector<uint8_t>& payload, MessageType type);
    void close();
    
    // Add client-specific methods here
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace client
} // namespace hwp

#endif // HWP_CLIENT_HPP 