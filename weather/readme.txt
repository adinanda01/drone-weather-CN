##Overview:
This project implements a real-time weather monitoring system where multiple weather stations (clients) distributed across a city send continuous weather data (temperature, humidity, air pressure, etc.) to a central server. The server processes this data and logs the information, while the client employs TCP Reno congestion control to adaptively manage data transmission rates under varying network conditions.

##File Structure:
server.cpp: Contains the server code that receives, decompresses, and processes weather data from clients.
client.cpp: Contains the client (weather station) code that collects weather data, compresses it, and sends it to the server using TCP Reno congestion control.

##Prerequisites:
Libraries:
Boost.Asio: For asynchronous network programming.
On Debian-based systems: sudo apt-get install libboost-all-dev
zlib: For data compression and decompression.
On Debian-based systems: sudo apt-get install zlib1g-dev
C++11 or later: For modern C++ features such as threading and random number generation.

##Server Code:
Compile the server code:
g++ -o server server.cpp -lboost_system -lz -lpthread
Run the server:
./server

#How It Works
The server listens on port 12345 for incoming connections from weather stations.
When a client connects and sends compressed weather data, the server decompresses the data and logs it.
The server handles multiple clients simultaneously using threads.
Configuration
Port: The server listens on port 12345. Modify the code to change the port if necessary.
Buffer Size: The buffer size for receiving data is set to 1024 bytes. Adjust this in the BUFFER_SIZE constant if needed.

#Key Functions
decompress(): Decompresses the incoming weather data using zlib.
handle_client(): Handles the connection with each client, receiving, decompressing, and logging data.

##Client Code:
Compile the client code:
g++ -o client client.cpp -lboost_system -lz -lpthread
Run the client:
./client

#How It Works
The client collects simulated weather data, compresses it using zlib, and sends it to the server.
The client uses TCP Reno to manage congestion control, adjusting the data transmission rate based on simulated network conditions.
The client simulates random weather data and network conditions to test the system's robustness.
#Configuration
Server Address and Port: The client connects to 127.0.0.1 on port 12345. Modify these values in the code to connect to a different server or port.
Congestion Control Parameters:
INITIAL_CWND: Initial congestion window size (default 1.0).
MAX_CWND: Maximum congestion window size (default 10.0).
INITIAL_SSTHRESH: Initial slow start threshold (default 8.0).
Data Simulation: The client generates random weather data with a uniform distribution.

#Key Functions
compress(): Compresses weather data before sending it to the server using zlib.
send_weather_data(): Simulates data transmission using TCP Reno, managing congestion control and simulating network conditions.