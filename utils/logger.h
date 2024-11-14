#pragma once
#include <boost/beast.hpp>
#include <iostream>

namespace beast = boost::beast;

namespace logger
{
	static void fail(beast::error_code ec, char const* what)
	{
		std::cerr 
			<< what 
			<< ": " 
			<< ec.message() 
			<< std::endl;
	}

	static void info(std::string_view what)
	{
		std::cout 
			<< what 
			<< std::endl;
	}

	static void error(std::string_view what)
	{
		std::cerr 
			<< "error: " 
			<< what 
			<< std::endl;
	}

	static std::string hide_context(const std::string& s)
	{
		const std::string context_prefix = (const char[12])"\"context\":\"";

		//replace contexrt with "...", as context is binary encoded being not human readable
		auto p = s.find(context_prefix);
		return
			p == std::string::npos
			? s 
			: s.substr(0, p + context_prefix.length() - 1) + "...}";
	}
};