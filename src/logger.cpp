#include "stdafx.h"
#include <algorithm>

#include "c11log/logger.h"

void c11log::logger::set_name(const std::string& name)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _logger_name = name;
}

const std::string& c11log::logger::get_name()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _logger_name;
}

void c11log::logger::add_sink(sink_ptr_t sink_ptr)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _sinks.push_back(sink_ptr);
}

void c11log::logger::remove_sink(sink_ptr_t sink_ptr)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _sinks.erase(std::remove(_sinks.begin(), _sinks.end(), sink_ptr), _sinks.end());
}

void c11log::logger::set_formatter(std::unique_ptr<formatters::formatter> formatter)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _formatter = std::move(formatter);
}

void c11log::logger::set_level(c11log::level::level_enum level)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _level = level;
}

bool c11log::logger::should_log(c11log::level::level_enum level)
{
    std::lock_guard<std::mutex> lock(_mutex);
    return level >= _level;
}

c11log::logger& c11log::get_logger(const std::string& name)
{
    return *(c11log::details::factory::instance().get_logger(name));
}