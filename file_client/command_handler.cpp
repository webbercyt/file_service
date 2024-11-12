#include "command_handler.h"
#include "websocket_session.h"
#include "binary_file_manager.h"
#include <boost/json.hpp>
#include <boost/uuid/uuid.hpp> 
#include <boost/uuid/uuid_generators.hpp> 
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <cassert>
#include <csignal>
#include <map>

namespace json = boost::json;

const std::string file_list = "-list";
const std::string file_all = "-all";
const std::string file_single = "-f=";

const std::map<std::string, std::string, std::less<>> scope_map_ = { 
    {file_list,     "list"}, 
    {file_all,      "all"}, 
    {file_single,   "single"} };

void fail(std::string_view what)
{
    std::cout << "error: " << what << "\n";
}

std::string get_random_uuid()
{
    return boost::uuids::to_string(boost::uuids::random_generator()());
}

command_handler::command_handler(
    std::shared_ptr<websocket_session> session, 
    std::shared_ptr<binary_file_manager> file_manager) :
        session_(session), file_manager_(file_manager)
{
    assert(session_ && "Websocket session must not be null");
    assert(file_manager_ && "File manager must not be null");
}

void command_handler::run()
{
    while (!stop_)
    {
        std::string command;
        std::getline(std::cin, command);
        handle(command);
    }
}

void command_handler::stop()
{
    stop_ = true;
}

void command_handler::handle(const std::string& command)
{
    if (command == "exit")
    {
        session_->close();
        stop_ = true;
        return;
    }

    auto space_pos = command.find_first_of(" ");
    if (space_pos == std::string::npos)
    {
        fail("unrecognized command.");
        return;
    }

    auto method = command.substr(0, space_pos);
    auto param = command.substr(space_pos + 1, command.length() - space_pos - 1);

    if (method == "get")
    {
        process_get_command(param);
    }
    else if (method == "post")
    {
        process_post_command(param);
    }
    else
    {
        fail("unrecognized command.");
    }
}

void command_handler::process_get_command(const std::string& param)
{
    if (param != file_list && 
        param != file_all && 
        param.find(file_single) == std::string::npos)
    {
        fail("unrecognized command.");
        return;
    }

    json::object obj;
    obj["uuid"] = get_random_uuid();
    obj["method"] = "get";

    if (param.find(file_single) != std::string::npos)
    {
        obj["scope"] = scope_map_.at(file_single);
        obj["target"] = get_file_name(param);
    }
    else
    {
        obj["scope"] = scope_map_.at(param);
    }

    session_->send(json::serialize(obj));
}

void command_handler::process_post_command(const std::string& param)
{
    if (param == file_all)
    {
        for (const auto& file : file_manager_->get_file_list())
        {
            send_post_command(file);
        }
    }
    else if (auto p = param.find(file_single); p != std::string::npos)
    {
        send_post_command(get_file_name(param));
    }
    else
    {
        fail("unrecognized command.");
        return;
    }
}

void command_handler::send_post_command(const std::string& file_name)
{
    json::object obj;
    obj["uuid"] = get_random_uuid();
    obj["method"] = "post";

    std::string context;
    if (!file_manager_->read(file_name, context))
    {
        return;
    }
    obj["target"] = file_name;
    obj["context"] = context;

    session_->send(json::serialize(obj));
}

std::string command_handler::get_file_name(std::string_view param) const
{
    auto p = param.find(file_single); 
    if (p == std::string::npos)
        return "";

    return std::string(
        param.substr(
            p + file_single.length(), 
            param.length() - p - file_single.length()));
}