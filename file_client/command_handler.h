#pragma once
#include <memory>
#include <string>
#include <atomic>

class websocket_session;
class binary_file_manager;

/*
support commands:

exit

get
	-list
	-all
	-f=[file name]

post
	-all
	-f=[file name]
*/

class command_handler
{
public:
	explicit command_handler(
		std::shared_ptr<websocket_session> session,
		std::shared_ptr<binary_file_manager> binary_file_manager);
	~command_handler() = default;

	void run();
	void stop();

private:
	void handle(const std::string& command);
	void process_get_command(const std::string& param);
	void process_post_command(const std::string& param);
	void send_post_command(const std::string& file_name);

	std::string get_file_name(std::string_view param) const;

	std::shared_ptr<websocket_session> session_;
	std::shared_ptr<binary_file_manager> file_manager_;
	std::atomic_bool stop_ = false;
};