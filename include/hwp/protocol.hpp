#ifndef HWP_PROTOCOL_HPP
#define HWP_PROTOCOL_HPP

#include <cstdint>
#include <vector>

namespace hwp {

// 协议版本
constexpr uint8_t PROTOCOL_VERSION = 1;

// 消息类型
enum class MessageType : uint8_t {
    HANDSHAKE = 0x01,
    DATA = 0x02,
    CONTROL = 0x03
};

// 协议标志
enum class Flags : uint8_t {
    NONE = 0x00,
    HTTP_MODE = 0x01,
    BINARY_MODE = 0x02
};

// 基础头部结构
struct BaseHeader {
    char magic[4];        // 魔数 "HWP\0"
    uint8_t version;      // 协议版本
    uint8_t flags;        // 标志位
    uint16_t head_len;    // 头部长度
};

// 会话头部结构
struct SessionHeader {
    uint32_t session_id;  // 会话ID
    uint32_t seq_num;     // 序列号
    uint32_t ack_num;     // 确认号
};

// 完整消息结构
struct Message {
    BaseHeader base_header;
    SessionHeader session_header;
    std::vector<uint8_t> payload;
};

// 协议处理器
class ProtocolHandler {
public:
    enum class ParseResult {
        HTTP,
        BINARY,
        ERROR
    };

    static Message create_message(MessageType type, uint32_t session_id,
                                const std::vector<uint8_t>& payload,
                                uint8_t flags);
    
    static std::vector<uint8_t> serialize_message(const Message& msg);
    
    ParseResult parse(const uint8_t* data, size_t length);
    const SessionHeader& get_session_header() const { return current_session_; }

private:
    SessionHeader current_session_;
};

} // namespace hwp

#endif // HWP_PROTOCOL_HPP 