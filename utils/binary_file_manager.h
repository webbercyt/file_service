#pragma once
#include <string>
#include <atomic>
#include <thread>
#include <queue>
#include <list>
#include <mutex>

class binary_file_manager
{
public:
	explicit binary_file_manager(const std::string& path);
	~binary_file_manager();

	bool read(std::string_view file_name, std::string& context) const;
	bool read(std::string_view file_name, std::string& context, std::string& error) const;
	bool write(const std::string& file_name, const std::string& context);
	bool write(const std::string& file_name, const std::string& context, std::string& error);

	std::list<std::string> get_file_list() const;
	void stop();
	
private:
	void create_root_folder() const;
	void do_write();
	
	std::string root_path_;

	std::atomic_bool stop_ = false;
	std::thread write_thread_;
	std::mutex write_mutex_;
	std::queue<std::pair<const std::string, const std::string>> write_queue_;
};