#pragma once
#include <string_view>

namespace text
{
	//infomation
	static const std::string connected_clients				= "connected client(s): ";
	static const std::string connect_closed					= "connnection closed";
	static const std::string connect_succeed				= "succeeded to connect ";
	static const std::string reconnecting					= "reconnecting ...";
	static const std::string websocket_handshake_accpted	= "websocket handshake accepted.";
	
	//error
	static const std::string cannot_read_no_name_file	= "cannot read file with empty name.";
	static const std::string fail_create_dir			= "failed to create directory: ";
	static const std::string fail_get_file_context		= "failed to get file context. file is opened by other process or path is invalid.";
	static const std::string fail_open_file				= "failed to open the file.";
	static const std::string fail_parse_message			= "failed to parse message";
	static const std::string filesystem_error			= "filesystem error: ";
	static const std::string file_manager_must_not_null	= "file manager must not be null";
	static const std::string miss_target_name			= "target name is missing";
	static const std::string session_must_not_null		= "session must not be null";
	static const std::string unrecognized_command		= "unrecognized command";
	static const std::string unrecognized_message		= "unrecognized message: ";
	static const std::string unsupported_scope			= "unsupported scope";
	
	//others
	static const std::string accept		= "accept";
	static const std::string bind		= "bind";
	static const std::string close		= "close";
	static const std::string connect	= "connect";
	static const std::string exit		= "exit";
	static const std::string get		= "get";
	static const std::string handshake	= "handshake";
	static const std::string listen		= "listen";
	static const std::string open		= "open";
	static const std::string post		= "post";
	static const std::string read		= "read";
	static const std::string received	= "received: ";
	static const std::string resolve	= "resolve";
	static const std::string sent		= "sent: ";
	static const std::string set_option = "set_option";
	static const std::string write		= "write";
}
