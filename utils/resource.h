#pragma once
#include <string_view>

namespace text
{
	//infomation
	static const std::string active_clients					= "active client(s): ";
	static const std::string connect_closed					= "succeeded to close";
	static const std::string connect_succeed				= "succeeded to connect ";
	static const std::string reconnecting					= "reconnecting ...";
	static const std::string websocket_handshake_accpted	= "websocket handshake accepted.";
	
	//error
	static const std::string cannot_read_no_name_file	= "cannot read file with empty name.";
	static const std::string fail_create_dir			= "failed to create directory: ";
	static const std::string fail_get_file_context		= "failed to get file context: file path is invalid or occupied by others.";
	static const std::string fail_open_file				= "failed to open file.";
	static const std::string fail_parse_message			= "failed to parse message";
	static const std::string filesystem_error			= "filesystem error: ";
	static const std::string file_over_max_size_limit	= "stop process as file size exceeds max limit: ";
	static const std::string file_manager_must_not_null	= "file manager must not be null";
	static const std::string miss_target_name			= "target name is missing";
	static const std::string session_must_not_null		= "session must not be null";
	static const std::string unrecognized_command		= "unrecognized command";
	static const std::string unrecognized_message		= "unrecognized message: ";
	static const std::string unsupported_scope			= "unsupported scope";
	static const std::string reach_max_support_client	= "reached max supported clients, reject to accept new client.";
	
	//others
	static const std::string accept		= "accept";
	static const std::string bind		= "bind";
	static const std::string close		= "close";
	static const std::string connect	= "connect";
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
	static const std::string x			= "x";
}

namespace ctrl
{
	static const uint64_t max_clients = 2048;
	static const std::uintmax_t max_file_size_in_mb = 10;
	static const std::uintmax_t max_file_size_in_byte = max_file_size_in_mb * 1024 * 1024;
}