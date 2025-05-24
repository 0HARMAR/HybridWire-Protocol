#ifndef HYBRIDWIRE_PROTOCOL_H
#define HYBRIDWIRE_PROTOCOL_H

#include <cstdint>
#include <string>
#include <vector>
#include <string_view>
#include <arpa/inet.h>

namespace hybridwire {

// Protocol version
constexpr uint8_t PROTOCOL_VERSION = 0x01;

// Protocol flags
enum class Flags : uint8_t {
    NONE = 0x00,
    HTTP_MODE = 0x01,        // HTTP模式
    BINARY_MODE = 0x02,      // 二进制模式
    COMPRESSED = 0x04,       // 数据压缩
    ENCRYPTED = 0x08,        // 数据加密
    REQUIRES_ACK = 0x10      // 需要确认
};

// Message types (for binary mode)
enum class MessageType : uint8_t {
    HANDSHAKE = 0x01,
    SESSION_INIT = 0x02,
    SESSION_ACK = 0x03,
    FILE_TRANSFER_START = 0x10,
    FILE_TRANSFER_DATA = 0x11,
    FILE_TRANSFER_END = 0x12,
    MESSAGE = 0x20,
    ERROR = 0xFF
};

// Protocol error codes
enum class ErrorCode : uint16_t {
    NO_ERROR = 0x0000,
    INVALID_PROTOCOL = 0x0001,
    INVALID_SESSION = 0x0002,
    AUTHENTICATION_FAILED = 0x0003,
    FILE_TRANSFER_ERROR = 0x0004,
    COMPRESSION_ERROR = 0x0005,
    ENCRYPTION_ERROR = 0x0006
};

#pragma pack(push, 1)
// Base protocol header (8 bytes, common for all modes)
struct BaseHeader {
    uint8_t magic[4];        // Magic number: "HWP\0"
    uint8_t version;         // Protocol version
    uint8_t flags;          // Protocol flags (HTTP/Binary mode, compression, encryption)
    uint16_t head_len;      // Total header length (network byte order)
};

// Session header (16 bytes, binary mode only)
struct SessionHeader {
    uint64_t session_id;     // Session identifier
    MessageType msg_type;    // Message type
    uint32_t payload_len;    // Length of the payload
    uint16_t reserved;       // Reserved for future use
};
#pragma pack(pop)

// Session state
struct SessionState {
    uint64_t session_id;
    bool is_authenticated;
    std::string client_id;
    uint64_t last_activity;
};

// File transfer state
struct FileTransferState {
    std::string filename;
    uint64_t total_size;
    uint64_t bytes_transferred;
    uint32_t chunk_size;
    bool is_complete;
};

// Message structure (binary mode)
struct Message {
    BaseHeader base_header;
    SessionHeader session_header;
    std::vector<uint8_t> payload;
};

// Protocol handler class
class ProtocolHandler {
public:
    enum class ParseResult { NEED_MORE, HTTP, BINARY, ERROR };

    static bool is_http_request(const std::vector<uint8_t>& data);
    static Message create_message(MessageType type, uint64_t session_id, 
                                const std::vector<uint8_t>& payload, uint8_t flags = 0);
    static std::vector<uint8_t> serialize_message(const Message& msg);
    static Message deserialize_message(const std::vector<uint8_t>& data);
    static FileTransferState initialize_file_transfer(const std::string& filename, uint64_t total_size);
    static SessionState create_session(const std::string& client_id);
    
    ParseResult parse(const uint8_t* data, size_t length);
    const BaseHeader& get_base_header() const { return current_message.base_header; }
    const SessionHeader& get_session_header() const { return current_message.session_header; }
    std::string_view get_http_data() const { return http_data; }

private:
    static uint64_t generate_session_id();
    static uint64_t get_current_timestamp();
    
    Message current_message;
    std::string http_data;
    size_t required_bytes = 0;
};

} // namespace hybridwire

#endif // HYBRIDWIRE_PROTOCOL_H 