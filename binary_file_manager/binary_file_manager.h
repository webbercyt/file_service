#pragma once
#include <string>
#include <list>

class binary_file_manager
{
public:
	explicit binary_file_manager(const std::string& path);
	~binary_file_manager() = default;

	bool read(std::string_view file_name, std::string& context) const;
	bool write(std::string_view file_name, std::string_view context) const;
	std::list<std::string> get_file_list() const;
	
private:
	void create_root_folder() const;

	std::string root_path_;
};