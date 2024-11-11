#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <queue>

namespace beast = boost::beast;         
namespace http = beast::http;           
namespace websocket = beast::websocket; 
namespace net = boost::asio;            
using tcp = boost::asio::ip::tcp;

class binary_file_manager;

// Sends a WebSocket message and prints the response
class websocket_session : public std::enable_shared_from_this<websocket_session>
{
public:
    // Resolver and socket require an io_context
    explicit websocket_session(net::io_context& ioc);

    // Start the asynchronous operation
    void run(
        char const* host, 
        char const* port, 
        std::shared_ptr<binary_file_manager> file_manager);

    void send(const std::string& s);

    void close();

private:
    void read();
    void reconnect();
    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
    void on_handshake(beast::error_code ec);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_close(beast::error_code ec);
    void on_send(const std::string& ss);
    void on_write(beast::error_code ec, std::size_t);
    void consume_read_buffer();

    tcp::resolver resolver_;
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string port_;
    std::queue<std::string> queue_;

    std::shared_ptr<binary_file_manager> file_manager_;
};