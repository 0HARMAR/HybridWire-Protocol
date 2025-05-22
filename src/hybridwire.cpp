#include "../include/hybridwire.hpp"
#include <algorithm>
#include <cstring>

using namespace HybridWire;

bool HybridWire::is_http_request(const std::vector<uint8_t>& data) {
    if (data.size() < 16) return false; // 最小合理HTTP请求长度
    
    // 检测常见HTTP方法
    auto has_http_method = [&](const char* method, size_t len) {
        return data.size() >= len && 
               std::equal(method, method + len, data.begin());
    };
    
    if (has_http_method("GET ", 4) || has_http_method("POST ", 5) ||
        has_http_method("HEAD ", 5) || has_http_method("PUT ", 4) ||
        has_http_method("DELETE ", 7)) {
        return true;
    }
    
    // 检测HTTP版本标识
    return std::search(data.begin(), data.end(), 
                      "HTTP/1.", "HTTP/1." + 7) != data.end();
}

ProtocolParser::ParseResult ProtocolParser::parse(const uint8_t* data, size_t length) {
    // 阶段1：解析协议头
    if (length < sizeof(ProtocolHeader)) {
        required_bytes_ = sizeof(ProtocolHeader);
        return ParseResult::NEED_MORE;
    }

    ProtocolHeader temp_header;
    memcpy(&temp_header, data, sizeof(ProtocolHeader));
    temp_header.head_len = ntohl(temp_header.head_len);

    // 验证协议魔数
    if (memcmp(temp_header.magic, "HWP", 3) != 0 || temp_header.magic[3] != '\0') {
        return ParseResult::ERROR;
    }

    // 检查协议版本
    if (temp_header.version != 0x01) {
        return ParseResult::ERROR;
    }

    // 阶段2：检查完整头部
    if (length < temp_header.head_len) {
        required_bytes_ = temp_header.head_len;
        return ParseResult::NEED_MORE;
    }

    // 阶段3：处理不同模式
    const uint8_t flags = temp_header.flags;
    const bool is_http = (flags & 0x01);
    const bool is_custom = (flags & 0x02);

    // 标志位冲突检测
    if (!(is_http ^ is_custom)) {
        return ParseResult::ERROR;
    }

    if (is_http) {
        // HTTP模式：验证后续数据
        const size_t http_start = temp_header.head_len;
        const size_t http_size = length - http_start;
        const uint8_t* http_data = data + http_start;
        
        std::vector<uint8_t> http_buffer(http_data, http_data + http_size);
        if (!is_http_request(http_buffer)) {
            return ParseResult::ERROR;
        }
        
        http_data_.assign(reinterpret_cast<const char*>(http_data), http_size);
        header_ = temp_header;
        return ParseResult::HTTP;
    } 
    else { // 自定义模式
        // 验证头部长度包含SessionHeader
        if (temp_header.head_len != sizeof(ProtocolHeader) + sizeof(SessionHeader)) {
            return ParseResult::ERROR;
        }

        // 解析会话头
        SessionHeader temp_session;
        memcpy(&temp_session, data + sizeof(ProtocolHeader), sizeof(SessionHeader));
        temp_session.session_id = be64toh(temp_session.session_id);
        temp_session.msg_type = ntohl(temp_session.msg_type);
        temp_session.payload_len = ntohl(temp_session.payload_len);

        // 检查负载完整性
        const size_t total_needed = temp_header.head_len + temp_session.payload_len;
        if (length < total_needed) {
            required_bytes_ = total_needed;
            return ParseResult::NEED_MORE;
        }

        header_ = temp_header;
        session_header_ = temp_session;
        return ParseResult::CUSTOM;
    }
}