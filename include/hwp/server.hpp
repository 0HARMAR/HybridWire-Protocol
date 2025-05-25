#ifndef HWP_SERVER_HPP
#define HWP_SERVER_HPP

#include <memory>
#include <boost/asio.hpp>

namespace hwp {
namespace server {

// Server-side functionality will be implemented here
class Server {
public:
    Server(boost::asio::io_context& io, unsigned short port);
    ~Server();
    
    // 禁用拷贝
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    
    // Add server-specific methods here
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace server
} // namespace hwp

#endif // HWP_SERVER_HPP 