#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <queue>
#include <boost/json.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// Report a failure
static void
fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Echoes back all received WebSocket messages
class session : public std::enable_shared_from_this<session>
{
public:
    // Take ownership of the socket
    explicit session(tcp::socket&& socket);

    // Get on the correct executor
    void run();

    // Send a message
    void send(const std::string& s);

private:
    // Start the asynchronous operation
    void on_run();

    void on_accept(beast::error_code ec);

    void do_read();
    void on_read(
            beast::error_code ec,
            std::size_t bytes_transferred);

    void on_write(
        beast::error_code ec,
        std::size_t bytes_transferred);

    void on_send(const std::string& s);

    void consume_buffer();

    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    std::queue<std::string> queue_;
};