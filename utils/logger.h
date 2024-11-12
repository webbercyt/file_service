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
};