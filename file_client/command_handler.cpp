#include "command_handler.h"
#include "session.h"
#include "binary_file_manager.h"
#include "messages.h"
#include "resource.h"
#include "logger.h"
#include <boost/json.hpp>
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

command_handler::command_handler(
    std::shared_ptr<session> session, 
    std::shared_ptr<binary_file_manager> file_manager) :
        session_(session), file_manager_(file_manager)
{
    assert(session_ && text::session_must_not_null.c_str());
    assert(file_manager_ && text::file_manager_must_not_null.c_str());
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
    if (command == text::exit)
    {
        session_->close();
        stop_ = true;
        return;
    }

    auto space_pos = command.find_first_of(" ");
    if (space_pos == std::string::npos)
    {
        logger::info(text::unrecognized_command);
        return;
    }

    auto method = command.substr(0, space_pos);
    auto param = command.substr(space_pos + 1, command.length() - space_pos - 1);

    if (method == text::get)
    {
        process_get_command(param);
    }
    else if (method == text::post)
    {
        process_post_command(param);
    }
    else
    {
        logger::info(text::unrecognized_command);
    }
}

void command_handler::process_get_command(const std::string& param)
{
    if (param != file_list && 
        param != file_all && 
        param.find(file_single) == std::string::npos)
    {
        logger::info(text::unrecognized_command);
        return;
    }

    get_message msg;
    if (param.find(file_single) != std::string::npos)
    {
        msg.scope_ = get_scope::e_gs_single;
        msg.target_ = get_file_name(param);
    }
    else if(param == file_all)
    {
        msg.scope_ = get_scope::e_gs_all;
    }

    session_->send(msg.serialize());
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
        logger::info(text::unrecognized_command);
        return;
    }
}

void command_handler::send_post_command(const std::string& file_name)
{
    post_message msg;
    std::string context;
    if (!file_manager_->read(file_name, context))
    {
        return;
    }
    msg.target_ = file_name;
    msg.context_ = context;

    session_->send(msg.serialize());
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