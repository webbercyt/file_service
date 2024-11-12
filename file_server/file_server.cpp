// file_server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "listener.h"
#include "binary_file_manager.h"
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace beast = boost::beast;         
namespace http = beast::http;           
namespace websocket = beast::websocket;
namespace net = boost::asio;       
using tcp = boost::asio::ip::tcp; 

int main(/*int argc, char** argv*/)
{
    /*if (argc != 5)
    {
        std::cerr <<
            "Usage: file_server <host> <port> <thread_number> <file_root_root>\n" <<
            "Example:\n" <<
            "    file_server 0.0.0.1 80 4 /usr/file\n" <<
            "    file_server 127.0.0.1 8080 4 c:/source/files\n";
        return EXIT_FAILURE;
    }*/

    // Check command line arguments.
    auto const address = net::ip::make_address("0.0.0.0");
    auto const port = static_cast<unsigned short>(8080);
    auto const threads = std::max<int>(1, 1);

    auto file_manager = std::make_shared<binary_file_manager>("C:/Users/cheny/source/repos/resources/server");

    // The io_context is required for all I/O
    net::io_context ioc{ threads };

    // Create and launch a listening port
    std::make_shared<listener>(ioc, tcp::endpoint{ address, port }, file_manager)->run();
    std::cout << "server is listening to port: " << port << std::endl;

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back(
            [&ioc]
            {
                ioc.run();
            });
    ioc.run();

    return EXIT_SUCCESS;
}