#include "../include/hybridwire_protocol.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace hybridwire;

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
    SessionState session = ProtocolHandler::createSession("client1");
    FileTransferState transferState = ProtocolHandler::initializeFileTransfer(filename, fileSize);

    // Send file transfer start message
    std::string metadata = filename + ":" + std::to_string(fileSize);
    std::vector<uint8_t> metadataPayload(metadata.begin(), metadata.end());
    
    Message startMsg = ProtocolHandler::createMessage(
        MessageType::FILE_TRANSFER_START,
        session.session_id,
        metadataPayload,
        static_cast<uint8_t>(Flags::REQUIRES_ACK)
    );

    // Serialize and send the message (in real implementation, this would go through network)
    std::vector<uint8_t> serializedStart = ProtocolHandler::serializeMessage(startMsg);
    std::cout << "Sending file transfer start message (" << serializedStart.size() << " bytes)\n";

    // Read and send file chunks
    std::vector<uint8_t> buffer(transferState.chunk_size);
    while (transferState.bytes_transferred < transferState.total_size) {
        // Read chunk from file
        file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        std::streamsize bytesRead = file.gcount();
        buffer.resize(bytesRead);

        // Create and send chunk message
        Message dataMsg = ProtocolHandler::createMessage(
            MessageType::FILE_TRANSFER_DATA,
            session.session_id,
            buffer
        );

        std::vector<uint8_t> serializedData = ProtocolHandler::serializeMessage(dataMsg);
        std::cout << "Sending file chunk (" << serializedData.size() << " bytes)\n";

        transferState.bytes_transferred += bytesRead;
        buffer.resize(transferState.chunk_size);
    }

    // Send file transfer end message
    Message endMsg = ProtocolHandler::createMessage(
        MessageType::FILE_TRANSFER_END,
        session.session_id,
        std::vector<uint8_t>(),
        static_cast<uint8_t>(Flags::REQUIRES_ACK)
    );

    std::vector<uint8_t> serializedEnd = ProtocolHandler::serializeMessage(endMsg);
    std::cout << "Sending file transfer end message (" << serializedEnd.size() << " bytes)\n";
    std::cout << "File transfer complete: " << transferState.bytes_transferred 
              << " bytes transferred\n";
}

int main() {
    try {
        demonstrateFileTransfer();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
} 