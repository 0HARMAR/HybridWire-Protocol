#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <arpa/inet.h>  // 用于字节序转换

namespace HybridWire {

#pragma pack(push, 1)
    // 协议头结构（二进制格式）
    struct ProtocolHeader {
        char magic[4];      // 协议标识 "HWP\0"
        uint32_t head_len;  // 头部长度（网络字节序）
        uint8_t  version;    // 协议版本 (0x01)
        uint8_t  flags;      // 标志位（0x01=HTTP模式, 0x02=二进制模式）
        uint16_t reserved;   // 保留字段
    };

    // 会话消息头（自定义协议模式）
    struct SessionHeader {
        uint64_t session_id; // 会话ID
        uint32_t msg_type;   // 消息类型
        uint32_t payload_len;// 数据长度
    };
#pragma pack(pop)

    // HTTP模式检测工具
    bool is_http_request(const std::vector<uint8_t>& data);

    // 协议解析器
    class ProtocolParser {
    public:
        enum class ParseResult { NEED_MORE, HTTP, CUSTOM, ERROR };

        ParseResult parse(const uint8_t* data, size_t length);

        // 获取解析结果
        const ProtocolHeader& header() const { return header_; }
        std::string_view http_data() const { return http_data_; }
        const SessionHeader& session_header() const { return session_header_; }

    private:
        ProtocolHeader header_;
        SessionHeader session_header_;
        std::string http_data_;
        size_t required_bytes_ = 0;
    };

} // namespace HybridWire