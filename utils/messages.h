#pragma once
#include <boost/json.hpp>
#include <boost/uuid/uuid.hpp> 
#include <boost/uuid/uuid_generators.hpp> 
#include <boost/uuid/uuid_io.hpp>

namespace json = boost::json;

enum class message_type { e_mt_unknown, e_mt_response, e_mt_get, e_mt_post};

struct response_message;
struct get_message;
struct post_message;
struct json_message_base
{
	std::string uuid_ = boost::uuids::to_string(boost::uuids::random_generator()());
	message_type type_ = message_type::e_mt_unknown;
	
	json_message_base() = default;

	explicit json_message_base(const json::object& obj)
	{
		if (obj.find("uuid") != obj.end())
			uuid_ = json::value_to<std::string>(obj.at("uuid"));
	}

	virtual ~json_message_base() = default;

	virtual void to_json(json::object& obj)
	{
		obj["uuid"] = uuid_;
	}

	static std::shared_ptr<json_message_base> parse(const std::string& s)
	{
		json::object obj;
		try
		{
			obj = json::parse(s).as_object();
		}
		catch (const boost::system::system_error&)
		{
			return nullptr;
		}

		if (obj.find("response") != obj.end())
		{
			return
				std::static_pointer_cast<json_message_base>(
					std::make_shared<response_message>(obj));
		}
		
		if (obj.find("method") != obj.end())
		{
			if (obj["method"] == "get")
				return
					std::static_pointer_cast<json_message_base>(
						std::make_shared<get_message>(obj));
			
			if (obj["method"] == "post")
				return
					std::static_pointer_cast<json_message_base>(
						std::make_shared<post_message>(obj));
		}

		return nullptr;
	}
};

/*
	<message example: response>
	{
		"uuid":"fd6b4eac-e386-4377-b4a5-0ec2cd182490"
		"response":"accepted"|"rejected"
		"reason":"reason of reject"
	}
*/
enum class response_type { e_rt_unknown, e_rt_accepted, e_rt_rejected };
struct response_message : json_message_base
{
	response_type response_ = response_type::e_rt_unknown;
	std::optional<std::string> reason_;

	using json_message_base::json_message_base;
	explicit response_message(const json::object& obj)
		: json_message_base(obj)
	{
		type_ = message_type::e_mt_response;

		if (obj.find("response") != obj.end())
		{
			auto response = json::value_to<std::string>(obj.at("response"));
			if (response == "accepted")
				response_ = response_type::e_rt_accepted;
			else if (response == "rejected")
				response_ = response_type::e_rt_rejected;
		}

		if (obj.find("reason") != obj.end())
			reason_ = json::value_to<std::string>(obj.at("reason"));
	}

	void to_json(json::object& obj) final
	{
		json_message_base::to_json(obj);
		
		switch (response_)
		{
		case response_type::e_rt_accepted:
			obj["response"] = "accepted";
			break;
		case response_type::e_rt_rejected:
			obj["response"] = "rejected";
			break;
		default:
			obj["response"] = "unknown";
			break;
		}

		if (reason_.has_value())
			obj["reason"] = reason_.value();
	}
};

/*
	<message example: get>
	{
		"uuid":"fd6b4eac-e386-4377-b4a5-0ec2cd182490"
		"method":"get"
		"scope":"single"|"all"
		"target":"test.txt"
	}
*/
enum class get_scope { e_gs_unknown, e_gs_single, e_gs_all };
struct get_message : json_message_base
{
	get_scope scope_ = get_scope::e_gs_unknown;
	std::optional<std::string> target_;

	using json_message_base::json_message_base;
	explicit get_message(const json::object& obj)
		: json_message_base(obj)
	{
		type_ = message_type::e_mt_get;

		if (obj.find("scope") == obj.end())
			return;

		if (obj.at("scope") == "all")
			scope_ = get_scope::e_gs_all;
		else if (obj.at("scope") == "single")
			scope_ = get_scope::e_gs_single;

		if (obj.find("target") != obj.end())
			target_ = json::value_to<std::string>(obj.at("target"));
	}

	void to_json(json::object& obj) final
	{
		json_message_base::to_json(obj);
		obj["method"] = "get";
		
		switch (scope_)
		{
		case get_scope::e_gs_single:
			obj["scope"] = "single";
			break;
		case get_scope::e_gs_all:
			obj["scope"] = "all";
			break;
		default:
			obj["scope"] = "unknown";
			break;
		}

		if (target_.has_value())
			obj["target"] = target_.value();
	}
};

/*
	<message example: post>
	{
		"uuid":"fd6b4eac-e386-4377-b4a5-0ec2cd182490"
		"method":"post"
		"target":"test.txt"
		"context":"context of file"
	}
*/
struct post_message : json_message_base
{
	std::string target_ = "";
	std::string context_ = "";

	using json_message_base::json_message_base;
	explicit post_message(const json::object& obj)
		: json_message_base(obj)
	{
		type_ = message_type::e_mt_post;

		if (obj.find("target") != obj.end())
			target_ = json::value_to<std::string>(obj.at("target"));
		if (obj.find("context") != obj.end())
			context_ = json::value_to<std::string>(obj.at("context"));
	}

	void to_json(json::object& obj) final
	{
		json_message_base::to_json(obj);
		obj["method"] = "post";
		obj["target"] = target_;
		obj["context"] = context_;
	}
};