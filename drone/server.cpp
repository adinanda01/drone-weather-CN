#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>

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

// Function to handle sending control commands to drones (UDP)
void handleControlCommands() {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CONTROL_PORT);
    inet_pton(AF_INET, "127.0.0.1", &client_addr.sin_addr); // Ensuring the IP is set correctly

    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        std::string command;
        std::cout << "Enter command to send to drones: ";
        std::getline(std::cin, command); // Get command input from user

        std::string encrypted_data = xorEncryptDecrypt(command, XOR_KEY); // Encrypt the command
        sendto(socket_fd, encrypted_data.c_str(), encrypted_data.size(), 0, (struct sockaddr*)&client_addr, addr_len); // Send the encrypted command to drones
    }

    close(socket_fd); // Close the socket
}

// Function to handle receiving telemetry data from drones (TCP)
void handleTelemetryData(int client_socket) {
    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            std::cerr << "Telemetry connection closed or error occurred" << std::endl;
            break;
        }

        std::string decrypted_data = xorEncryptDecrypt(std::string(buffer, bytes_received), XOR_KEY); // Decrypt the received telemetry data
        std::cout << "Telemetry Data Received: " << decrypted_data << std::endl; 
    }

    close(client_socket); // Close the client socket
}

// Function to handle receiving files from drones (simulated QUIC using TCP)
void handleFileTransfers(int client_socket) {
    char buffer[BUFFER_SIZE];
    std::ofstream output_file("received_file", std::ios::binary); // Open file to save the received data

    if (!output_file.is_open()) {
        std::cerr << "Failed to open file for writing!" << std::endl;
        close(client_socket);
        return;
    }

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            std::cerr << "Connection closed or error occurred" << std::endl;
            break;
        }

        output_file.write(buffer, bytes_received); // Write received data to the file
    }

    std::cout << "File received and saved as 'received_file'" << std::endl;
    output_file.close();

    // Print the content of the received file
    std::ifstream file("received_file", std::ios::binary);
    if (file.is_open()) {
        std::cout << "Content of 'received_file':\n";
        std::cout << file.rdbuf(); // Print file content to console
        file.close();
    } else {
        std::cerr << "Failed to open received file for reading." << std::endl;
    }

    close(client_socket); // Close the client socket
}

// Function to start the server and manage incoming connections
void startServer() {
    // Control Commands (UDP)
    std::thread controlThread(handleControlCommands);
    controlThread.detach();

    // Telemetry Data (TCP)
    int telemetry_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in telemetry_addr;
    telemetry_addr.sin_family = AF_INET;
    telemetry_addr.sin_addr.s_addr = INADDR_ANY;
    telemetry_addr.sin_port = htons(TELEMETRY_PORT);

    bind(telemetry_socket, (struct sockaddr*)&telemetry_addr, sizeof(telemetry_addr));
    listen(telemetry_socket, 5); // Listen for incoming telemetry connections

    // File Transfers (TCP)
    int file_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in file_addr;
    file_addr.sin_family = AF_INET;
    file_addr.sin_addr.s_addr = INADDR_ANY;
    file_addr.sin_port = htons(FILE_PORT);

    bind(file_socket, (struct sockaddr*)&file_addr, sizeof(file_addr));
    listen(file_socket, 5); // Listen for incoming file transfer connections

    while (true) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(telemetry_socket, &readfds);
        FD_SET(file_socket, &readfds);
        int max_fd = std::max(telemetry_socket, file_socket) + 1;

        int activity = select(max_fd, &readfds, nullptr, nullptr, nullptr); // Monitor sockets for incoming data
        if (activity < 0) {
            std::cerr << "Select error" << std::endl;
            break;
        }

        if (FD_ISSET(telemetry_socket, &readfds)) {
            int client_socket = accept(telemetry_socket, nullptr, nullptr);
            std::thread telemetryThread(handleTelemetryData, client_socket); // Handle telemetry data in a separate thread
            telemetryThread.detach();
        }

        if (FD_ISSET(file_socket, &readfds)) {
            int client_socket = accept(file_socket, nullptr, nullptr);
            std::thread fileThread(handleFileTransfers, client_socket); // Handle file transfer in a separate thread
            fileThread.detach();
        }
    }
}

int main() {
    startServer();
    return 0;
}
