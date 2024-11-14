#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <thread>
#include <memory>
#include <string>
#include <queue>
#include <mutex>
#include <atomic>

namespace beast = boost::beast;         
namespace http = beast::http;           
namespace websocket = beast::websocket; 
namespace net = boost::asio;            
using tcp = boost::asio::ip::tcp;       

struct json_message_base;
class binary_file_manager;
class session : public std::enable_shared_from_this<session>
{
public:
    // Take ownership of the socket
    explicit session(
        tcp::socket&& socket,
        std::shared_ptr<binary_file_manager> file_manager);
    ~session();

    // Get on the correct executor
    void run();

    // Send a message
    void send(const std::string& s);

    static uint64_t clients();

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
    void process_get_message(std::shared_ptr<json_message_base> msg);
    void process_post_message(std::shared_ptr<json_message_base> msg);

    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    std::queue<std::string> queue_;
    std::shared_ptr<binary_file_manager> file_manager_;


    std::atomic_bool stop_send_ = false;
    std::thread send_thread_;

    static uint64_t clients_;
    static std::mutex clients_mutex_;
};