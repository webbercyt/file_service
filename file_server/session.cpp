#include "session.h"
#include "binary_file_manager.h"
#include "messages.h"
#include "resource.h"
#include "logger.h"
#include <cassert>

uint64_t session::clients_ = 0;
std::mutex session::clients_mutex_;

session::session(tcp::socket&& socket, std::shared_ptr<binary_file_manager> file_manager)
    : ws_(std::move(socket)), file_manager_(file_manager)
{
    assert(file_manager && text::file_manager_must_not_null.c_str());
    std::scoped_lock<std::mutex>lock(clients_mutex_);
    clients_++;
    logger::info(text::connected_clients + std::to_string(clients_));
}

session::~session()
{
    std::scoped_lock<std::mutex>lock(clients_mutex_);
    clients_--;
    logger::info(text::connected_clients + std::to_string(clients_));
}

// Get on the correct executor
void session::run()
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
void session::on_run()
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

void session::on_accept(beast::error_code ec)
{
    if (ec)
        return logger::fail(ec, text::accept.c_str());

    logger::info(text::websocket_handshake_accpted);

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
        return logger::fail(ec, text::read.c_str());

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
        return logger::fail(ec, text::write.c_str());

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
    auto data = beast::buffers_to_string(buffer_.data());
    buffer_.consume(buffer_.size());

    //server only processes "get" and "post"
    switch (
        auto msg = json_message_base::parse(data);
    msg->type_)
    {
    case message_type::e_mt_get:
    {
        logger::info(text::received + data);
        process_get_message(msg);
        break;
    }
    case message_type::e_mt_post:
    {
        //log message received without context, as context could be long
        const std::string context_prefix = (const char[12])"\"context\":\"";
        auto p = data.find(context_prefix);
        p == std::string::npos ?
            logger::info(text::received + data) :
            logger::info(text::received + data.substr(0, p + context_prefix.length() - 1) + "...}");

        process_post_message(msg);
        break;
    }
    default:
        logger::error(text::unrecognized_message + data);
        break;
    }
}

void session::process_get_message(std::shared_ptr<json_message_base> msg)
{
    auto get_msg = std::dynamic_pointer_cast<get_message>(msg);
    if (!get_msg)
        return;

    std::string reason = "";
    bool succesed = false;
    switch (get_msg->scope_)
    {
    case get_scope::e_gs_single:
    {
        if (get_msg->target_.has_value())
        {
            std::string context;
            if (file_manager_->read(get_msg->target_.value(), context, reason))
            {
                post_message post_msg;
                post_msg.target_ = get_msg->target_.value();
                post_msg.context_ = context;
                send(post_msg.serialize());
                succesed = true;
            }
        }
        else
        {
            reason = text::miss_target_name;
        }

        break;
    }
    case get_scope::e_gs_all:
    {
        succesed = true;
        for (const auto& file_name : file_manager_->get_file_list())
        {
            post_message post_msg;
            std::string context;
            if (!file_manager_->read(file_name, context))
                continue;
            post_msg.target_ = file_name;
            post_msg.context_ = context;
            send(post_msg.serialize());
        }
        break;
    }
    default:
        reason = text::unsupported_scope;
        break;
    }

    //send response
    response_message rsp_msg;
    rsp_msg.uuid_ = msg->uuid_;
    rsp_msg.response_ =
        succesed
        ? response_type::e_rt_accepted
        : response_type::e_rt_rejected;
    if (!reason.empty())
        rsp_msg.reason_ = reason;
    send(rsp_msg.serialize());
}

void session::process_post_message(std::shared_ptr<json_message_base> msg)
{
    bool succesed = false;
    std::string reason;
    if (auto post_msg = std::dynamic_pointer_cast<post_message>(msg))
    {
        succesed = (file_manager_->write(
            post_msg->target_,
            post_msg->context_,
            reason));
    }
    else
    {
        reason = text::fail_parse_message;
    }

    //send response
    response_message rsp_msg;
    rsp_msg.uuid_ = msg->uuid_;
    rsp_msg.response_ =
        succesed
        ? response_type::e_rt_accepted
        : response_type::e_rt_rejected;
    if (!reason.empty())
        rsp_msg.reason_ = reason;
    send(rsp_msg.serialize());
}