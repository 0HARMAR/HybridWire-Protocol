#include "../include/hybridwire_protocol.h"
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

using namespace hybridwire;

// Helper function to print binary data
void print_hex(const std::vector<uint8_t>& data) {
    for (uint8_t byte : data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << std::endl;
}

// Example of file transfer using the protocol
void demonstrateFileTransfer() {
    // Initialize file transfer
    const std::string filename = "example.txt";
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    uint64_t fileSize = file.tellg();
    file.seekg(0);

    // Create a session
    SessionState session = ProtocolHandler::create_session("client1");
    FileTransferState transferState = ProtocolHandler::initialize_file_transfer(filename, fileSize);

    // Send file transfer start message
    std::string metadata = filename + ":" + std::to_string(fileSize);
    std::vector<uint8_t> metadataPayload(metadata.begin(), metadata.end());
    
    // 组合标志位
    uint8_t startFlags = static_cast<uint8_t>(Flags::BINARY_MODE) | 
                        static_cast<uint8_t>(Flags::REQUIRES_ACK);
    
    Message startMsg = ProtocolHandler::create_message(
        MessageType::FILE_TRANSFER_START,
        session.session_id,
        metadataPayload,
        startFlags
    );

    // Serialize and send the message (in real implementation, this would go through network)
    std::vector<uint8_t> serializedStart = ProtocolHandler::serialize_message(startMsg);
    std::cout << "Sending file transfer start message (" << serializedStart.size() << " bytes)\n";

    // Read and send file chunks
    std::vector<uint8_t> buffer(transferState.chunk_size);
    while (transferState.bytes_transferred < transferState.total_size) {
        // Read chunk from file
        file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        std::streamsize bytesRead = file.gcount();
        buffer.resize(bytesRead);

        // Create and send chunk message
        Message dataMsg = ProtocolHandler::create_message(
            MessageType::FILE_TRANSFER_DATA,
            session.session_id,
            buffer,
            static_cast<uint8_t>(Flags::BINARY_MODE)
        );

        std::vector<uint8_t> serializedData = ProtocolHandler::serialize_message(dataMsg);
        std::cout << "Sending file chunk (" << serializedData.size() << " bytes)\n";

        transferState.bytes_transferred += bytesRead;
        buffer.resize(transferState.chunk_size);
    }

    // Send file transfer end message
    uint8_t endFlags = static_cast<uint8_t>(Flags::BINARY_MODE) | 
                      static_cast<uint8_t>(Flags::REQUIRES_ACK);
    
    Message endMsg = ProtocolHandler::create_message(
        MessageType::FILE_TRANSFER_END,
        session.session_id,
        std::vector<uint8_t>(),
        endFlags
    );

    std::vector<uint8_t> serializedEnd = ProtocolHandler::serialize_message(endMsg);
    std::cout << "Sending file transfer end message (" << serializedEnd.size() << " bytes)\n";
    std::cout << "File transfer complete: " << transferState.bytes_transferred 
              << " bytes transferred\n";
}

// Test Wire mode functionality
void test_wire_mode() {
    std::cout << "\n=== Testing Wire Mode ===\n" << std::endl;

    // Create a test message
    std::string test_payload = "Hello, Wire Mode!";
    std::vector<uint8_t> payload(test_payload.begin(), test_payload.end());
    
    // Create a new session
    auto session = ProtocolHandler::create_session("test_client");
    std::cout << "Created session with ID: " << session.session_id << std::endl;

    // Create and serialize a message
    auto msg = ProtocolHandler::create_message(
        MessageType::MESSAGE,
        session.session_id,
        payload
    );

    std::cout << "Created message with type: " 
              << static_cast<int>(msg.session_header.msg_type) << std::endl;

    // Serialize the message
    auto serialized = ProtocolHandler::serialize_message(msg);
    std::cout << "Serialized message (" << serialized.size() << " bytes):" << std::endl;
    print_hex(serialized);

    // Deserialize and verify
    auto deserialized = ProtocolHandler::deserialize_message(serialized);
    std::string received_payload(deserialized.payload.begin(), deserialized.payload.end());
    
    std::cout << "\nDeserialized message:" << std::endl;
    std::cout << "- Session ID: " << deserialized.session_header.session_id << std::endl;
    std::cout << "- Message Type: " 
              << static_cast<int>(deserialized.session_header.msg_type) << std::endl;
    std::cout << "- Payload: " << received_payload << std::endl;

    // Test file transfer initialization
    auto file_transfer = ProtocolHandler::initialize_file_transfer("test.txt", 1024);
    std::cout << "\nInitialized file transfer:" << std::endl;
    std::cout << "- Filename: " << file_transfer.filename << std::endl;
    std::cout << "- Total size: " << file_transfer.total_size << " bytes" << std::endl;
    std::cout << "- Chunk size: " << file_transfer.chunk_size << " bytes" << std::endl;
}

int main() {
    try {
        demonstrateFileTransfer();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    test_wire_mode();
    return 0;
} 