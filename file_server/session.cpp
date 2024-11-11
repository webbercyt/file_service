#include "session.h"

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
        return fail(ec, "accept");

    {
        std::cout << "accept websocket handshake" << std::endl;
    }

    // Read a message
    do_read();
}

void
session::do_read()
{
    // Read a message into our buffer
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &session::on_read,
            shared_from_this()));
}

void
session::on_read(
    beast::error_code ec,
    std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // This indicates that the session was closed
    if (ec == websocket::error::closed)
        return;

    if (ec)
        return fail(ec, "read");

    // Echo the message
    ws_.text(ws_.got_text());

    boost::json::object response;
    {
        auto data = beast::buffers_to_string(buffer_.data());
        auto obj = boost::json::parse(data).as_object();

        if (obj.find("uuid") != obj.end())
        {
            response["uuid"] = obj["uuid"];
            response["response"] = "accepted";
        }
    }
    std::string b = boost::json::serialize(response);

    ws_.async_write(
        /*buffer_.data(),*/net::buffer("response"),
        /*net::buffer(b),*/
        beast::bind_front_handler(
            &session::on_write,
            shared_from_this()));

    //ws_.text(ws_.got_text());
//                //{
//                //    auto data = beast::buffers_to_string(buffer_.data());
//                //    auto obj = boost::json::parse(data).as_object();
//
//                //    boost::json::object response;
//                //    if (obj.find("uuid") != obj.end())
//                //    {
//                //        response["uuid"] = obj["uuid"];
//                //        response["response"] = "accepted";
//                //        //ws_.write(net::buffer();
//                //    }
//
//                //    /*{
//                //        obj["target"] = "response.txt";
//                //        ws_.write(net::buffer(boost::json::serialize(obj)));
//                //    }*/
//
//                //    ws_.async_write(
//                //        net::buffer(boost::json::serialize(response)),
//                //        beast::bind_front_handler(
//                //            &session::on_write,
//                //            shared_from_this()));
//                //}
}

void
session::on_write(
    beast::error_code ec,
    std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return fail(ec, "write");

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Do another read
    do_read();
}