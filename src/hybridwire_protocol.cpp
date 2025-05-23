#include "../include/hybridwire_protocol.h"
#include <cstring>
#include <chrono>
#include <stdexcept>
#include <arpa/inet.h>

namespace hybridwire {

namespace {
    const uint8_t MAGIC_NUMBER[4] = {'H', 'W', 'P', '\0'};
    constexpr size_t MAX_PAYLOAD_SIZE = 16 * 1024 * 1024; // 16MB
    constexpr size_t DEFAULT_CHUNK_SIZE = 64 * 1024; // 64KB
}

Message ProtocolHandler::createMessage(MessageType type, uint32_t session_id, 
                                    const std::vector<uint8_t>& payload, uint8_t flags) {
    if (payload.size() > MAX_PAYLOAD_SIZE) {
        throw std::runtime_error("Payload size exceeds maximum allowed size");
    }

    Message msg;
    std::memcpy(msg.header.magic, MAGIC_NUMBER, sizeof(MAGIC_NUMBER));
    msg.header.version = PROTOCOL_VERSION;
    msg.header.type = type;
    msg.header.flags = flags;
    msg.header.session_id = htonl(session_id);
    msg.header.payload_length = htonl(static_cast<uint32_t>(payload.size()));
    msg.payload = payload;

    return msg;
}

std::vector<uint8_t> ProtocolHandler::serializeMessage(const Message& msg) {
    std::vector<uint8_t> serialized;
    serialized.reserve(sizeof(ProtocolHeader) + msg.payload.size());

    // Serialize header
    const uint8_t* header_ptr = reinterpret_cast<const uint8_t*>(&msg.header);
    serialized.insert(serialized.end(), header_ptr, header_ptr + sizeof(ProtocolHeader));

    // Append payload
    serialized.insert(serialized.end(), msg.payload.begin(), msg.payload.end());

    return serialized;
}

Message ProtocolHandler::deserializeMessage(const std::vector<uint8_t>& data) {
    if (data.size() < sizeof(ProtocolHeader)) {
        throw std::runtime_error("Invalid message: too small");
    }

    Message msg;
    std::memcpy(&msg.header, data.data(), sizeof(ProtocolHeader));

    // Validate magic number
    if (std::memcmp(msg.header.magic, MAGIC_NUMBER, sizeof(MAGIC_NUMBER)) != 0) {
        throw std::runtime_error("Invalid message: wrong magic number");
    }

    // Convert network byte order to host byte order
    msg.header.session_id = ntohl(msg.header.session_id);
    msg.header.payload_length = ntohl(msg.header.payload_length);

    // Extract payload
    if (data.size() != sizeof(ProtocolHeader) + msg.header.payload_length) {
        throw std::runtime_error("Invalid message: payload size mismatch");
    }

    msg.payload.assign(
        data.begin() + sizeof(ProtocolHeader),
        data.end()
    );

    return msg;
}

FileTransferState ProtocolHandler::initializeFileTransfer(const std::string& filename, uint64_t total_size) {
    FileTransferState state;
    state.filename = filename;
    state.total_size = total_size;
    state.bytes_transferred = 0;
    state.chunk_size = DEFAULT_CHUNK_SIZE;
    state.is_complete = false;
    return state;
}

SessionState ProtocolHandler::createSession(const std::string& client_id) {
    SessionState state;
    state.session_id = generateSessionId();
    state.is_authenticated = false;
    state.client_id = client_id;
    state.last_activity = getCurrentTimestamp();
    return state;
}

uint32_t ProtocolHandler::generateSessionId() {
    // Simple implementation - in production, use a more secure method
    return static_cast<uint32_t>(std::chrono::system_clock::now()
        .time_since_epoch().count() & 0xFFFFFFFF);
}

uint64_t ProtocolHandler::getCurrentTimestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

} // namespace hybridwire