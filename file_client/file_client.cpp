// file_client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "websocket_session.h"
#include "command_handler.h"
#include "binary_file_manager.h"
#include <thread>
#include <csignal>

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        std::cerr <<
            "Usage: file-client <host> <port> <file_root_root>\n" <<
            "Example:\n" <<
            "    file-client 0.0.0.1 80 /usr/file\n" <<
            "    file-client 127.0.0.1 8080 c:/source/files\n";
        return EXIT_FAILURE;
    }

    auto const host = argv[1];
    auto const port = argv[2];
    auto const path = argv[3];

    auto file_manager_ = std::make_shared<binary_file_manager>(path);

    // The io_context is required for all I/O
    net::io_context ioc;
    auto session_ = std::make_shared<websocket_session>(ioc);

    //start command handler
    std::thread command(
        [&session_, &file_manager_]
        {
            command_handler(session_, file_manager_).run();
        });

    // Run the I/O service. The call will return when the socket is closed.
    session_->run(
        host,
        port,
        file_manager_);
    ioc.run();

    if (command.joinable())
        command.join();

    return EXIT_SUCCESS;
}