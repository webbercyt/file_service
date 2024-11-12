#include "listener.h"
#include "binary_file_manager.h"
#include "resource.h"
#include "logger.h"

listener::listener(
    net::io_context& ioc,
    tcp::endpoint endpoint,
    std::shared_ptr<binary_file_manager> file_manager)
    : ioc_(ioc)
    , acceptor_(ioc)
    , file_manager_(file_manager)
{
    beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if (ec)
    {
        logger::fail(ec, text::open.c_str());
        return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec)
    {
        logger::fail(ec, text::set_option.c_str());
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if (ec)
    {
        logger::fail(ec, text::bind.c_str());
        return;
    }

    // Start listening for connections
    acceptor_.listen(
        net::socket_base::max_listen_connections, ec);
    if (ec)
    {
        logger::fail(ec, text::listen.c_str());
        return;
    }
}

// Start accepting incoming connections
void listener::run()
{
    do_accept();
}

void listener::do_accept()
{
    // The new connection gets its own strand
    acceptor_.async_accept(
        net::make_strand(ioc_),
        beast::bind_front_handler(
            &listener::on_accept,
            shared_from_this()));
}

void listener::on_accept(beast::error_code ec, tcp::socket socket)
{
    if (ec)
    {
        logger::fail(ec, text::accept.c_str());
    }
    else
    {
        // Create the session and run it
        if (session::clients() + 1 > ctrl::max_clients)
        {
            logger::info(text::reach_max_support_client);
            return;
        }

        std::make_shared<session>(std::move(socket), file_manager_)->run();
    }

    // Accept another connection
    do_accept();
}