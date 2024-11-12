#pragma once
#include <string_view>

namespace text
{
	//infomation
	static const std::string connected_clients = "connected client(s): ";
	static const std::string websocket_handshake_accpted = "websocket handshake accepted.";

	//error
	static const std::string cannot_read_no_name_file = "cannot read file with empty name.";
	static const std::string fail_create_dir = "failed to create directory: ";
	static const std::string fail_get_file_context = "failed to get file context. file is opened by other process or path is invalid.";
	static const std::string fail_open_file = "failed to open the file.";
	static const std::string fail_parse_message = "failed to parse message";
	static const std::string filesystem_error = "filesystem error: ";
	static const std::string unsupported_scope = "unsupported scope";
	static const std::string miss_target_name = "target name is missing";
	static const std::string file_manager_must_not_null = "file manager must not be null";
	static const std::string unrecognized_message = "unrecognized message: ";
	
	
	//others
	static const std::string accept = "accept";
	static const std::string read = "read";
	static const std::string write = "write";
	static const std::string received = "received: ";
	

}
