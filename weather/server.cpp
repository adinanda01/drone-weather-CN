#include <iostream>
#include <boost/asio.hpp>
#include <zlib.h>
#include <thread>
#include <chrono>
#include <iomanip> // For std::fixed and std::setprecision

using boost::asio::ip::tcp;

const int BUFFER_SIZE = 1024;

// Function to decompress data using zlib
std::string decompress(const std::string &data) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (inflateInit(&zs) != Z_OK)
        throw std::runtime_error("inflateInit failed");

    zs.next_in = (Bytef*)data.data();
    zs.avail_in = data.size();

    int ret;
    char outbuffer[BUFFER_SIZE];
    std::string outstring;

    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = inflate(&zs, 0);

        if (outstring.size() < zs.total_out)
            outstring.append(outbuffer, zs.total_out - outstring.size());
    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END)
        throw std::runtime_error("Exception during zlib decompression");

    return outstring;
}

// Function to handle a client connection
void handle_client(tcp::socket socket) {
    try {
        std::array<char, BUFFER_SIZE> buffer;
        boost::system::error_code error;
        while (true) {
            size_t len = socket.read_some(boost::asio::buffer(buffer), error);

            if (error == boost::asio::error::eof)
                break; // Connection closed by client
            else if (error)
                throw boost::system::system_error(error);

            std::string compressed_data(buffer.data(), len);
            std::string data = decompress(compressed_data);

            // Log received data
            std::cout << "Received weather data: " << data << std::endl;

            // Simulate processing delay
            std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Added delay for simulation
        }
    } catch (std::exception& e) {
        std::cerr << "Exception in client handler: " << e.what() << std::endl;
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12345));

        std::cout << "Server is listening on port 12345..." << std::endl;

        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            // Start a new thread for each client connection
            std::thread(handle_client, std::move(socket)).detach();
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
