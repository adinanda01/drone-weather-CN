#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <thread>

constexpr int CONTROL_PORT = 8081; // Port for Control Commands (UDP)
constexpr int TELEMETRY_PORT = 8082; // Port for Telemetry Data (TCP)
constexpr int FILE_PORT = 8083; // Port for File Transfers (simulated QUIC using TCP)
constexpr int BUFFER_SIZE = 4096; // Buffer size for data transmission
constexpr char XOR_KEY = 'K'; // XOR key for encryption and decryption

// Function to encrypt/decrypt data using XOR cipher
std::string xorEncryptDecrypt(const std::string& data, char key) {
    std::string result = data;
    for (size_t i = 0; i < data.size(); i++) {
        result[i] = data[i] ^ key;
    }
    return result;
}

// Function to receive control commands from the server (UDP)
void receiveControlCommands() {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    char buffer[BUFFER_SIZE];

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(CONTROL_PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)); // Bind to the control port

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recvfrom(socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, &addr_len);
        if (bytes_received > 0) {
            std::string decrypted_data = xorEncryptDecrypt(std::string(buffer, bytes_received), XOR_KEY); // Decrypt the received command
            std::cout << "Control Command Received: " << decrypted_data << std::endl;
        }
    }

    close(socket_fd);
}

// Function to send telemetry data to the server (TCP)
void sendTelemetryData(const std::string& server_ip) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TELEMETRY_PORT);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Telemetry connection failed" << std::endl;
        close(socket_fd);
        return;
    }

    while (true) {
        std::string telemetry;
        std::cout << "Enter telemetry data: ";
        std::getline(std::cin, telemetry); // Get telemetry data input from user

        std::string encrypted_data = xorEncryptDecrypt(telemetry, XOR_KEY); // Encrypt the telemetry data
        send(socket_fd, encrypted_data.c_str(), encrypted_data.size(), 0); // Send encrypted telemetry data to the server
    }

    close(socket_fd);
}

// Function to send a file to the server (TCP)
void sendFile(const std::string& server_ip, const std::string& file_path) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(FILE_PORT);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "File transfer connection failed" << std::endl;
        close(socket_fd);
        return;
    }

    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        close(socket_fd);
        return;
    }

    char buffer[BUFFER_SIZE];
    while (file.read(buffer, BUFFER_SIZE) || file.gcount() > 0) {
        send(socket_fd, buffer, file.gcount(), 0); // Send file data to the server
    }

    std::cout << "File sent successfully!" << std::endl;
    file.close();
    close(socket_fd);
}

int main() {
    std::string server_ip = "127.0.0.1";

    // Start receiving control commands in a separate thread
    std::thread commandThread(receiveControlCommands);
    commandThread.detach();

    // Start thread to send telemetry data
    std::thread telemetryThread(sendTelemetryData, server_ip);
    telemetryThread.detach();
    // Send a file to the server
    sendFile(server_ip, "test.txt");

    // Keep the client running to continue receiving commands
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
