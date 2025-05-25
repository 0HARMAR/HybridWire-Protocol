#include "../include/hwp/protocol.hpp"
#include <cstring>
#include <arpa/inet.h>

namespace hwp {

Message ProtocolHandler::create_message(MessageType type, uint32_t session_id,
                                     const std::vector<uint8_t>& payload,
                                     uint8_t flags) {
    Message msg;
    
    // 设置基础头部
    msg.base_header.magic[0] = 'H';
    msg.base_header.magic[1] = 'W';
    msg.base_header.magic[2] = 'P';
    msg.base_header.magic[3] = '\0';
    msg.base_header.version = PROTOCOL_VERSION;
    msg.base_header.flags = flags;
    msg.base_header.head_len = sizeof(BaseHeader) + sizeof(SessionHeader);
    
    // 设置会话头部
    msg.session_header.session_id = session_id;
    msg.session_header.seq_num = 0;  // TODO: 实现序列号管理
    msg.session_header.ack_num = 0;  // TODO: 实现确认号管理
    
    // 设置负载
    msg.payload = payload;
    
    return msg;
}

std::vector<uint8_t> ProtocolHandler::serialize_message(const Message& msg) {
    std::vector<uint8_t> buffer;
    size_t total_size = sizeof(BaseHeader) + sizeof(SessionHeader) + msg.payload.size();
    buffer.reserve(total_size);
    
    // 序列化基础头部
    const uint8_t* base_header_ptr = reinterpret_cast<const uint8_t*>(&msg.base_header);
    buffer.insert(buffer.end(), base_header_ptr, base_header_ptr + sizeof(BaseHeader));
    
    // 序列化会话头部
    const uint8_t* session_header_ptr = reinterpret_cast<const uint8_t*>(&msg.session_header);
    buffer.insert(buffer.end(), session_header_ptr, session_header_ptr + sizeof(SessionHeader));
    
    // 添加负载
    buffer.insert(buffer.end(), msg.payload.begin(), msg.payload.end());
    
    return buffer;
}

ProtocolHandler::ParseResult ProtocolHandler::parse(const uint8_t* data, size_t length) {
    if (length < sizeof(BaseHeader)) {
        return ParseResult::ERROR;
    }
    
    const BaseHeader* base_header = reinterpret_cast<const BaseHeader*>(data);
    
    // 验证魔数
    if (strncmp(base_header->magic, "HWP", 3) != 0) {
        return ParseResult::ERROR;
    }
    
    // 检查协议版本
    if (base_header->version != PROTOCOL_VERSION) {
        return ParseResult::ERROR;
    }
    
    // 根据标志判断协议类型
    if (base_header->flags & static_cast<uint8_t>(Flags::HTTP_MODE)) {
        return ParseResult::HTTP;
    } else if (base_header->flags & static_cast<uint8_t>(Flags::BINARY_MODE)) {
        if (length >= sizeof(BaseHeader) + sizeof(SessionHeader)) {
            // 保存会话信息
            current_session_ = *reinterpret_cast<const SessionHeader*>(data + sizeof(BaseHeader));
            return ParseResult::BINARY;
        }
    }
    
    return ParseResult::ERROR;
}

} // namespace hwp