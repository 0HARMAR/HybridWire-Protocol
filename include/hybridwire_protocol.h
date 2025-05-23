#ifndef HYBRIDWIRE_PROTOCOL_H
#define HYBRIDWIRE_PROTOCOL_H

#include <cstdint>
#include <string>
#include <vector>

namespace hybridwire {

// Protocol version
constexpr uint8_t PROTOCOL_VERSION = 0x01;

// Message types
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

// Protocol flags
enum class Flags : uint8_t {
    NONE = 0x00,
    COMPRESSED = 0x01,
    ENCRYPTED = 0x02,
    REQUIRES_ACK = 0x04
};

// Protocol header structure (16 bytes)
#pragma pack(push, 1)
struct ProtocolHeader {
    uint8_t magic[4];           // Magic number: "HWP\0"
    uint8_t version;            // Protocol version
    MessageType type;           // Message type
    uint8_t flags;             // Message flags
    uint32_t session_id;        // Session identifier
    uint32_t payload_length;    // Length of the payload
};
#pragma pack(pop)

// Session state
struct SessionState {
    uint32_t session_id;
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

// Message structure
struct Message {
    ProtocolHeader header;
    std::vector<uint8_t> payload;
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

// Protocol handler class
class ProtocolHandler {
public:
    static Message createMessage(MessageType type, uint32_t session_id, 
                               const std::vector<uint8_t>& payload, uint8_t flags = 0);
    static std::vector<uint8_t> serializeMessage(const Message& msg);
    static Message deserializeMessage(const std::vector<uint8_t>& data);
    static FileTransferState initializeFileTransfer(const std::string& filename, uint64_t total_size);
    static SessionState createSession(const std::string& client_id);

private:
    static uint32_t generateSessionId();
    static uint64_t getCurrentTimestamp();
};

} // namespace hybridwire

#endif // HYBRIDWIRE_PROTOCOL_H 