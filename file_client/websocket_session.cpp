#include "websocket_session.h"
#include "binary_file_manager.h"
#include "logger.h"
#include "messages.h"
#include <cassert>

websocket_session::websocket_session(net::io_context& ioc)
    : resolver_(net::make_strand(ioc))
    , ws_(net::make_strand(ioc))
{
}

void websocket_session::run(char const* host, char const* port, std::shared_ptr<binary_file_manager> file_manager)
{
    assert(file_manager && "File manager must not be null");
    file_manager_ = file_manager;

    // Save these for later
    host_ = host;
    port_ = port;

    // Look up the domain name
    resolver_.async_resolve(
        host,
        port,
        beast::bind_front_handler(
            &websocket_session::on_resolve,
            shared_from_this()));
}

void websocket_session::send(const std::string& s)
{
    const std::string context_prefix = (const char[12])"\"context\":\"";

    //log message sent without context, as context could be long
    auto p = s.find(context_prefix);
    p == std::string::npos ?
        logger::info("sent: " + s) :
        logger::info("sent: " + s.substr(0, p + context_prefix.length() - 1) + "...}");

    net::post(
        ws_.get_executor(),
        beast::bind_front_handler(
            &websocket_session::on_send,
            shared_from_this(),
            s));
}

void websocket_session::reconnect()
{
    std::cout << "reconnecting ...\n";
    //reconnect in 10s
    std::this_thread::sleep_for(std::chrono::seconds(10));

    resolver_.async_resolve(
        host_,
        port_,
        beast::bind_front_handler(
            &websocket_session::on_resolve,
            shared_from_this()));
}

void websocket_session::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
{
    if (ec)
        return logger::fail(ec, "resolve");

    // Set the timeout for the operation
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(ws_).async_connect(
        results,
        beast::bind_front_handler(
            &websocket_session::on_connect,
            shared_from_this()));
}

void websocket_session::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
{
    if (ec)
    {
        logger::fail(ec, "connect");
        reconnect();
        return;
    }

    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(ws_).expires_never();

    // Set suggested timeout settings for the websocket
    ws_.set_option(
        websocket::stream_base::timeout::suggested(
            beast::role_type::client));

    // Set a decorator to change the User-Agent of the handshake
    ws_.set_option(websocket::stream_base::decorator(
        [](websocket::request_type& req)
        {
            req.set(http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) +
                " websocket-client-async");
        }));

    // Update the host_ string. This will provide the value of the
    // Host HTTP header during the WebSocket handshake.
    // See https://tools.ietf.org/html/rfc7230#section-5.4
    auto host = host_ + ':' + std::to_string(ep.port());

    // Perform the websocket handshake
    ws_.async_handshake(host, "/",
        beast::bind_front_handler(
            &websocket_session::on_handshake,
            shared_from_this()));
}

void websocket_session::on_handshake(beast::error_code ec)
{
    if (ec)
        return logger::fail(ec, "handshake");

    std::cout << "succeeded to connect " << host_ << ":" << port_ << std::endl;

    // read a message into buffer
    read();
}

void websocket_session::read()
{
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &websocket_session::on_read,
            shared_from_this()));
}

void websocket_session::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return logger::fail(ec, "read");

    //consume buffer
    consume_read_buffer();

    // send the message
    read();
}

void websocket_session::on_send(const std::string& s)
{
    // Always add to queue
    queue_.push(s);

    // Are we already writing?
    if (queue_.size() > 1)
        return;

    // We are not currently writing, so send this immediately
    ws_.async_write(
        net::buffer(queue_.front()),
        beast::bind_front_handler(
            &websocket_session::on_write,
            shared_from_this()));
}

void websocket_session::on_write(beast::error_code ec, std::size_t)
{
    // Handle the error, if any
    if (ec)
        return logger::fail(ec, "write");

    // Remove the string from the queue
    queue_.pop();

    // send the next message if any
    if (!queue_.empty())
        ws_.async_write(
            net::buffer(queue_.front()),
            beast::bind_front_handler(
                &websocket_session::on_write,
                shared_from_this()));
}

void websocket_session::consume_read_buffer()
{
    auto data = beast::buffers_to_string(buffer_.data());
    buffer_.consume(buffer_.size());

    //client only processes "response" and "post"
    switch (
        auto msg = json_message_base::parse(data);
    msg->type_)
    {
    case message_type::e_mt_response:
        logger::info("received: " + data);
        break;
    case message_type::e_mt_post:
        if (auto post_msg = std::dynamic_pointer_cast<post_message>(msg))
            file_manager_->write(
                post_msg->target_,
                post_msg->context_);
        break;
    default:
        logger::error("unrecognized message: " + data);
        break;
    }
}

void websocket_session::close()
{
    ws_.async_close(websocket::close_code::normal,
        beast::bind_front_handler(
            &websocket_session::on_close,
            shared_from_this()));
}

void websocket_session::on_close(beast::error_code ec)
{
    if (ec)
        return logger::fail(ec, "close");

    std::cout << "connnection closed\n";
}