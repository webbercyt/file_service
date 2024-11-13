#include "binary_file_manager.h"
#include "logger.h"
#include "resource.h"
#include <boost/algorithm/hex.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

binary_file_manager::binary_file_manager(const std::string& path)
	: root_path_(path)
{
	create_root_folder();
	write_thread_ = std::thread(&binary_file_manager::do_write, this);
}

binary_file_manager::~binary_file_manager()
{
	if (write_thread_.joinable())
		write_thread_.join();
}

bool binary_file_manager::read(std::string_view file_name, std::string& context) const
{
	std::string error;
	return read(file_name, context, error);
}

bool binary_file_manager::read(std::string_view file_name, std::string& context, std::string& error) const
{
	if (file_name.empty())
	{
		error = text::cannot_read_no_name_file;
		logger::error(error);
		return false;
	}

	std::string path = root_path_;
	path.append("/");
	path.append(file_name);

	try
	{
		if (!std::filesystem::exists(path))
		{
			error = text::fail_get_file_context;
			logger::error(error);
			return false;
		}

		if (std::uintmax_t file_size = std::filesystem::file_size(path); 
			file_size > ctrl::max_file_size_in_byte)
		{ 
			error = 
				text::file_over_max_size_limit 
				+ std::to_string(ctrl::max_file_size_in_mb) 
				+ " mb";
			logger::error(error);
			return false;
		}

		std::ifstream file(path, std::ios::binary);
		if (!file)
		{
			error = text::fail_get_file_context;
			logger::error(error);
			return false;
		}
		
		std::ostringstream oss;
		oss << file.rdbuf();
		file.close();

		std::string data = oss.str();
		std::string encoded_context;
		boost::algorithm::hex(data.begin(), data.end(), std::back_inserter(encoded_context));

		context = encoded_context;
		return true;
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		error = text::filesystem_error + std::string(e.what());
		logger::error(error);
	}
	
	return false;
}

bool binary_file_manager::write(const std::string& file_name, const std::string& context)
{
	std::string error;
	return write(file_name, context, error);
}

bool binary_file_manager::write(const std::string& file_name, const std::string& context, std::string& error)
{
	if (file_name.empty())
	{
		error = text::cannot_read_no_name_file;
		logger::error(error);
		return false;
	}

	std::scoped_lock lock(write_mutex_);
	write_queue_.push(std::make_pair(file_name, context));
	return true;
}

std::list<std::string> binary_file_manager::get_file_list() const
{
	std::list<std::string> file_list;
	for (const auto& entry : std::filesystem::directory_iterator(root_path_))
	{
		if (std::filesystem::is_regular_file(entry.status()))
		{
			file_list.push_back(entry.path().filename().string());
		}
	}
	return file_list;
}

void binary_file_manager::create_root_folder() const
{
	try
	{
		if (std::filesystem::exists(root_path_))
			return;

		if (!std::filesystem::create_directory(root_path_))
		{
			logger::error(text::fail_create_dir + root_path_);
		}
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		logger::error(text::filesystem_error + std::string(e.what()));
	}
}

void binary_file_manager::do_write()
{
	while (!stop_)
	{
		std::pair<std::string, std::string> data 
			= std::make_pair("", "");

		{
			std::scoped_lock lock(write_mutex_);
			if (!write_queue_.empty())
			{
				data = write_queue_.front();
				write_queue_.pop();
			}
		}

		if (data.first.empty())
		{
			//todo: wait for signal to break sleep
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			continue;
		}

		std::string path = root_path_;
		path.append("/");
		path.append(data.first);

		try
		{
			std::ofstream out_file(path, std::ios::binary);
			if (!out_file.is_open())
			{
				logger::error(text::fail_open_file);
				continue;
			}

			std::string decoded_context;
			boost::algorithm::unhex(
				data.second.begin(), 
				data.second.end(), 
				std::back_inserter(decoded_context));

			out_file.write(decoded_context.c_str(), decoded_context.size());
			out_file.close();
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			logger::error(text::filesystem_error + std::string(e.what()));
		}
	}
}

void binary_file_manager::stop()
{
	stop_ = true;
}