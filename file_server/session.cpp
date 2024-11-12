#include "session.h"
#include "logger.h"

session::session(tcp::socket&& socket)
    : ws_(std::move(socket))
{
}

// Get on the correct executor
void
session::run()
{
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    net::dispatch(ws_.get_executor(),
        beast::bind_front_handler(
            &session::on_run,
            shared_from_this()));
}

// Start the asynchronous operation
void
session::on_run()
{
    // Set suggested timeout settings for the websocket
    ws_.set_option(
        websocket::stream_base::timeout::suggested(
            beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    ws_.set_option(websocket::stream_base::decorator(
        [](websocket::response_type& res)
        {
            res.set(http::field::server,
                std::string(BOOST_BEAST_VERSION_STRING) +
                " websocket-server-async");
        }));
    // Accept the websocket handshake
    ws_.async_accept(
        beast::bind_front_handler(
            &session::on_accept,
            shared_from_this()));
}

void
session::on_accept(beast::error_code ec)
{
    if (ec)
        return logger::fail(ec, "accept");

    logger::info("websocket handshake accepted.");

    // Read a message
    do_read();
}

void session::do_read()
{
    // Read a message into our buffer
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &session::on_read,
            shared_from_this()));
}

void session::on_read(
    beast::error_code ec,
    std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // This indicates that the session was closed
    if (ec == websocket::error::closed)
        return;

    if (ec)
        return logger::fail(ec, "read");

    consume_buffer();

    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &session::on_read,
            shared_from_this()));
}

void session::on_write(
    beast::error_code ec,
    std::size_t)
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
                &session::on_write,
                shared_from_this()));
}

void session::send(const std::string& s)
{
    net::post(
        ws_.get_executor(),
        beast::bind_front_handler(
            &session::on_send,
            shared_from_this(),
            s));
}

void session::on_send(const std::string& s)
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
            &session::on_write,
            shared_from_this()));
}

void session::consume_buffer()
{
    //server only processes "get" and "post"
    auto data = beast::buffers_to_string(buffer_.data());
    buffer_.consume(buffer_.size());

    try
    {
        auto obj = boost::json::parse(data).as_object();
        
        boost::json::object response;
        if (obj.find("uuid") != obj.end())
        {
            response["uuid"] = obj["uuid"];
            response["response"] = "accepted";
            send(boost::json::serialize(response));
        }
        
        if (obj.find("method") != obj.end() &&
            obj["method"] == "post" &&
            obj.find("target") != obj.end() &&
            obj.find("context") != obj.end())
        {
            /*file_manager_->write(
                boost::json::value_to<std::string>(obj["target"]),
                boost::json::value_to<std::string>(obj["context"]));*/

            obj["target"] = "server_response.txt";
            send(boost::json::serialize(obj));
            return;
        }
    }
    catch (const boost::system::system_error& e)
    {
        std::cerr << e.what() << std::endl;
    }

    logger::info("unrecognized message: " + data);
}