#include "stdafx.h"
#include "c11log/details/factory.h"
#include "c11log/logger.h"

c11log::details::factory::logger_ptr c11log::details::factory::get_logger(const std::string &name)
{
	std::lock_guard<std::mutex> lock(_loggers_mutex);
	auto found = _loggers.find(name);
	if (found == _loggers.end()) {
		auto new_logger_ptr = std::make_shared<c11log::logger>(name);
		_loggers.insert(std::make_pair(name, new_logger_ptr));
		return new_logger_ptr;
	}
	else {		
		return found->second;
	}
}

c11log::details::factory & c11log::details::factory::instance()
{
	static c11log::details::factory instance;

	return instance;
}
