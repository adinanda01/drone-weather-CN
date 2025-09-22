#include <iostream>
#include <boost/asio.hpp>
#include <zlib.h>
#include <thread>
#include <chrono>
#include <random>
#include <iomanip> // For std::fixed and std::setprecision

using boost::asio::ip::tcp;

const int BUFFER_SIZE = 1024;

// TCP Reno parameters
const double INITIAL_CWND = 1.0; // Initial congestion window
const double MAX_CWND = 10.0; // Maximum congestion window
const double INITIAL_SSTHRESH = 8.0; // Initial slow start threshold
const double INCREASE_FACTOR = 1.0; // Additive increase factor for congestion avoidance

// Function to compress data using zlib
std::string compress(const std::string &data) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK)
        throw std::runtime_error("deflateInit failed");

    zs.next_in = (Bytef*)data.data();
    zs.avail_in = data.size();

    int ret;
    char outbuffer[BUFFER_SIZE];
    std::string outstring;

    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out)
            outstring.append(outbuffer, zs.total_out - outstring.size());
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END)
        throw std::runtime_error("Exception during zlib compression");

    return outstring;
}

// Function to simulate sending weather data with TCP Reno congestion control
void send_weather_data(tcp::socket& socket) {
    try {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 5); // Simulate random weather data

        double cwnd = INITIAL_CWND; // Congestion window
        double ssthresh = INITIAL_SSTHRESH; // Slow start threshold

        while (true) {
            // Simulated weather data
            std::string weather_data = "Temperature: " + std::to_string(dis(gen)) + "°C, Humidity: " + std::to_string(dis(gen)) + "%";

            // Compress the weather data
            std::string compressed_data = compress(weather_data);

            // Send data in chunks according to the congestion window (cwnd)
            for (int i = 0; i < cwnd; ++i) {
                boost::system::error_code ignored_error;
                socket.write_some(boost::asio::buffer(compressed_data), ignored_error);

                // Simulate RTT and logging
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Added delay for better visualization
                std::cout << "Sent data chunk" << " with cwnd=" << cwnd << std::endl;
            }

            // Simulate an ACK reception and update cwnd and ssthresh
            if (cwnd < ssthresh) {
                // Slow Start
                cwnd *= 2;
                std::cout << "Slow start: increased cwnd to " << cwnd << std::endl;
            } else {
                // Congestion Avoidance
                cwnd += INCREASE_FACTOR;
                std::cout << "Congestion avoidance: increased cwnd to " << cwnd << std::endl;
            }

            // Simulate congestion detection (loss detected)
            if (rand() % 5 > 3) { // Example condition for congestion detection
                std::cout << "Simulated congestion detected!" << std::endl;
                ssthresh = cwnd / 2;
                cwnd = INITIAL_CWND; // Reset to initial cwnd after loss
                std::cout << "Fast retransmit: cwnd reset to " << cwnd << ", ssthresh set to " << ssthresh << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1)); // Delay to simulate network conditions
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::query query("127.0.0.1", "12345");
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoint_iterator);

        send_weather_data(socket);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
