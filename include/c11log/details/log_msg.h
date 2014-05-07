#pragma once

namespace c11log
{
namespace details
{
struct log_msg
{
    log_msg() = default;
    log_msg(level::level_enum l):
		msg_level(l),
		msg_time(),
		msg_header_size(0),
		str() {}


	log_msg(const log_msg& other):
		msg_level(other.msg_level),
		msg_time(other.msg_time),
		msg_header_size(other.msg_header_size),
		str(other.str) {}

	log_msg(log_msg&& other):log_msg()
	{
		swap(*this, other);
	}

	friend void swap(log_msg& l, log_msg& r)
	{
		using std::swap;
		swap(l.msg_level, r.msg_level);
		swap(l.msg_time, r.msg_time);
		swap(l.msg_header_size, r.msg_header_size);
		swap(l.str, r.str);
	}



	log_msg& operator=(log_msg other)
	{
		swap(*this, other);
		return *this;
	}


	void clear()
	{
		msg_header_size = 0;
		str.clear();		
	}

    level::level_enum msg_level;	
	log_clock::time_point msg_time;
    std::size_t msg_header_size;
	std::string str;

};
}
}
