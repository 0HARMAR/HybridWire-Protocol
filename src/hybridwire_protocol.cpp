#include "../include/hybridwire_protocol.h"
#include <cstring>
#include <chrono>
#include <stdexcept>
#include <arpa/inet.h>
#include <algorithm>

namespace hybridwire {

namespace {
    const uint8_t MAGIC_NUMBER[4] = {'H', 'W', 'P', '\0'};
    constexpr size_t MAX_PAYLOAD_SIZE = 16 * 1024 * 1024; // 16MB
    constexpr size_t DEFAULT_CHUNK_SIZE = 64 * 1024; // 64KB
    constexpr size_t MIN_HTTP_SIZE = 16; // 最小合理HTTP请求长度
}

bool ProtocolHandler::is_http_request(const std::vector<uint8_t>& data) {
    if (data.size() < MIN_HTTP_SIZE) return false;
    
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

Message ProtocolHandler::create_message(MessageType type, uint64_t session_id, 
                                     const std::vector<uint8_t>& payload, uint8_t flags) {
    if (payload.size() > MAX_PAYLOAD_SIZE) {
        throw std::runtime_error("Payload size exceeds maximum allowed size");
    }

    Message msg;
    // 设置基础头部
    std::memcpy(msg.base_header.magic, MAGIC_NUMBER, sizeof(MAGIC_NUMBER));
    msg.base_header.version = PROTOCOL_VERSION;
    msg.base_header.flags = flags;
    msg.base_header.head_len = htons(sizeof(BaseHeader) + sizeof(SessionHeader));

    // 设置会话头部
    msg.session_header.session_id = session_id;
    msg.session_header.msg_type = type;
    msg.session_header.payload_len = static_cast<uint32_t>(payload.size());
    msg.session_header.reserved = 0;

    msg.payload = payload;
    return msg;
}

std::vector<uint8_t> ProtocolHandler::serialize_message(const Message& msg) {
    std::vector<uint8_t> serialized;
    const size_t total_size = sizeof(BaseHeader) + sizeof(SessionHeader) + msg.payload.size();
    serialized.reserve(total_size);

    // 序列化基础头部
    const uint8_t* base_ptr = reinterpret_cast<const uint8_t*>(&msg.base_header);
    serialized.insert(serialized.end(), base_ptr, base_ptr + sizeof(BaseHeader));

    // 序列化会话头部
    const uint8_t* session_ptr = reinterpret_cast<const uint8_t*>(&msg.session_header);
    serialized.insert(serialized.end(), session_ptr, session_ptr + sizeof(SessionHeader));

    // 添加负载
    serialized.insert(serialized.end(), msg.payload.begin(), msg.payload.end());

    return serialized;
}

Message ProtocolHandler::deserialize_message(const std::vector<uint8_t>& data) {
    if (data.size() < sizeof(BaseHeader)) {
        throw std::runtime_error("Invalid message: too small for base header");
    }

    Message msg;
    size_t offset = 0;

    // 解析基础头部
    std::memcpy(&msg.base_header, data.data(), sizeof(BaseHeader));
    offset += sizeof(BaseHeader);

    // 验证魔数
    if (std::memcmp(msg.base_header.magic, MAGIC_NUMBER, sizeof(MAGIC_NUMBER)) != 0) {
        throw std::runtime_error("Invalid message: wrong magic number");
    }

    // 如果是二进制模式，解析会话头部
    if (msg.base_header.flags & static_cast<uint8_t>(Flags::BINARY_MODE)) {
        if (data.size() < offset + sizeof(SessionHeader)) {
            throw std::runtime_error("Invalid message: too small for session header");
        }

        std::memcpy(&msg.session_header, data.data() + offset, sizeof(SessionHeader));
        offset += sizeof(SessionHeader);

        // 网络字节序转换
        msg.session_header.session_id = be64toh(msg.session_header.session_id);
        msg.session_header.payload_len = ntohl(msg.session_header.payload_len);

        // 验证负载长度
        if (data.size() != offset + msg.session_header.payload_len) {
            throw std::runtime_error("Invalid message: payload size mismatch");
        }
    }

    // 提取负载
    msg.payload.assign(data.begin() + offset, data.end());

    return msg;
}

ProtocolHandler::ParseResult ProtocolHandler::parse(const uint8_t* data, size_t length) {
    // 阶段1：解析基础头部
    if (length < sizeof(BaseHeader)) {
        required_bytes = sizeof(BaseHeader);
        return ParseResult::NEED_MORE;
    }

    BaseHeader temp_header;
    std::memcpy(&temp_header, data, sizeof(BaseHeader));
    uint16_t head_len = ntohs(temp_header.head_len);

    // 验证魔数和版本
    if (std::memcmp(temp_header.magic, MAGIC_NUMBER, sizeof(MAGIC_NUMBER)) != 0 ||
        temp_header.version != PROTOCOL_VERSION) {
        return ParseResult::ERROR;
    }

    // 阶段2：检查完整头部
    if (length < head_len) {
        required_bytes = head_len;
        return ParseResult::NEED_MORE;
    }

    // 保存基础头部
    current_message.base_header = temp_header;

    // 根据模式处理
    if (temp_header.flags & static_cast<uint8_t>(Flags::HTTP_MODE)) {
        // HTTP模式
        const uint8_t* http_start = data + sizeof(BaseHeader);
        const size_t http_size = length - sizeof(BaseHeader);
        
        std::vector<uint8_t> http_buffer(http_start, http_start + http_size);
        if (!is_http_request(http_buffer)) {
            return ParseResult::ERROR;
        }
        
        http_data.assign(reinterpret_cast<const char*>(http_start), http_size);
        return ParseResult::HTTP;
    } 
    else if (temp_header.flags & static_cast<uint8_t>(Flags::BINARY_MODE)) {
        // 二进制模式
        if (head_len < sizeof(BaseHeader) + sizeof(SessionHeader)) {
            return ParseResult::ERROR;
        }

        // 解析会话头部
        std::memcpy(&current_message.session_header, 
                   data + sizeof(BaseHeader), 
                   sizeof(SessionHeader));

        // 网络字节序转换
        current_message.session_header.session_id = be64toh(current_message.session_header.session_id);
        current_message.session_header.payload_len = ntohl(current_message.session_header.payload_len);

        // 检查完整性
        const size_t total_needed = head_len + current_message.session_header.payload_len;
        if (length < total_needed) {
            required_bytes = total_needed;
            return ParseResult::NEED_MORE;
        }

        // 提取负载
        current_message.payload.assign(
            data + head_len,
            data + head_len + current_message.session_header.payload_len
        );

        return ParseResult::BINARY;
    }

    return ParseResult::ERROR;
}

FileTransferState ProtocolHandler::initialize_file_transfer(const std::string& filename, uint64_t total_size) {
    FileTransferState state;
    state.filename = filename;
    state.total_size = total_size;
    state.bytes_transferred = 0;
    state.chunk_size = DEFAULT_CHUNK_SIZE;
    state.is_complete = false;
    return state;
}

SessionState ProtocolHandler::create_session(const std::string& client_id) {
    SessionState state;
    state.session_id = generate_session_id();
    state.is_authenticated = false;
    state.client_id = client_id;
    state.last_activity = get_current_timestamp();
    return state;
}

uint64_t ProtocolHandler::generate_session_id() {
    // 使用64位会话ID
    return std::chrono::system_clock::now()
        .time_since_epoch().count();
}

uint64_t ProtocolHandler::get_current_timestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

} // namespace hybridwire