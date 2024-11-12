#include "binary_file_manager.h"
#include "logger.h"
#include <boost/algorithm/hex.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>

binary_file_manager::binary_file_manager(const std::string& path)
	: root_path_(path)
{
	create_root_folder();
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
		logger::error("cannot read file with empty name");
		return false;
	}

	std::string path = root_path_;
	path.append("/");
	path.append(file_name);

	try
	{
		if (std::ifstream file(path, std::ios::binary); file)
		{
			std::ostringstream oss;
			oss << file.rdbuf();
			file.close();

			std::string data = oss.str();
			std::string encoded_context;
			boost::algorithm::hex(data.begin(), data.end(), std::back_inserter(encoded_context));

			context = encoded_context;
			return true;
		}

		logger::error("failed to get file context. file is opened by other process or path is invalid.");
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		logger::error("filesystem error: " + std::string(e.what()));
	}
	
	return false;
}

bool binary_file_manager::write(std::string_view file_name, std::string_view context) const
{
	std::string error;
	return write(file_name, context, error);
}

bool binary_file_manager::write(std::string_view file_name, std::string_view context, std::string& error) const
{
	if (file_name.empty())
	{
		logger::error("cannot read file with empty name");
		return false;
	}

	std::string path = root_path_;
	path.append("/");
	path.append(file_name);

	try
	{
		std::ofstream out_file(path, std::ios::binary);
		if (!out_file.is_open())
		{
			logger::error("failed to open the file.");
			return false;
		}

		std::string decoded_context;
		boost::algorithm::unhex(context.begin(), context.end(), std::back_inserter(decoded_context));

		out_file.write(decoded_context.c_str(), decoded_context.size());
		out_file.close();

		return true;
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		logger::error("filesystem error: " + std::string(e.what()));
	}
	
	return false;
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
			logger::error("failed to create directory: " + root_path_);
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		logger::error("filesystem error: " + std::string(e.what()));
	}
}