##Overview
This project implements a communication system for a fleet of autonomous drones, supporting three types of communication:

Control Commands (UDP): Allows the server to send control commands to the drones.
Telemetry Data (TCP): Drones send telemetry data back to the server.
File Transfers (Simulated QUIC using TCP): Drones can transfer files to the server.
The communication channels are encrypted using an XOR cipher for simplicity.

##Prerequisites
C++ Compiler (GCC recommended)
A Unix-like environment (Linux/MacOS)

##How to Run
Server
Compile the server code:
g++ -o server server.cpp -pthread
Run the server:
./server
The server listens for control commands on UDP port 8081, telemetry data on TCP port 8082, and file transfers on TCP port 8083.
Input control commands directly into the console.

Client
Compile the client code:
g++ -o client client.cpp -pthread
Run the client:
./client

Input telemetry data directly into the console.

##Implementation:
The project was implemented in C++ using socket programming. The system is designed to run on Unix-like environments, and the code is multithreaded to handle concurrent communication channels efficiently.

#XOR Encryption
To ensure data security, all communications are encrypted using a simple XOR cipher with a fixed key ('K'). This method was chosen for its simplicity and to demonstrate basic encryption concepts.

#Control Commands
The server sends control commands to the drones via UDP. These commands are encrypted before transmission and decrypted upon receipt by the drone.

#Telemetry Data
Telemetry data is transmitted from the drone to the server using TCP. This data is critical for monitoring the status and performance of the drones.

#File Transfers
Files are transferred from drones to the server over TCP, with the server saving the files locally. The file transfer mechanism simulates the QUIC protocol, emphasizing reliable and efficient data transfer.
