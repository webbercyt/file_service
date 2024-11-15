#pragma once
#include "session.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class binary_file_manager;
// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;

public:
    listener(
        net::io_context& ioc,
        tcp::endpoint endpoint,
        std::shared_ptr<binary_file_manager> file_manager);

    // Start accepting incoming connections
    void run();

private:
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);

    std::shared_ptr<binary_file_manager> file_manager_;
};