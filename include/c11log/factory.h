#pragma once
#include <memory>
#include "logger.h"
#include "sinks/file_sinks.h"
#include "sinks/stdout_sinks.h"

//
// logger creation shotcuts
//
namespace c11log
{
namespace factory
{

//
// console loggers single/multi threaded
//
std::unique_ptr<logger> stdout_logger(const std::string& name = "")
{
    auto sink = std::make_shared<sinks::stderr_sink_st>();
    return std::unique_ptr<logger>(new logger(name, sink));
}

std::unique_ptr<logger> stdout_logger_mt(const std::string& name = "")
{
    auto sink = std::make_shared<sinks::stderr_sink_mt>();
    return std::unique_ptr<logger>(new logger(name, sink));
}

//
// simple file logger single/multi threaded
//
std::unique_ptr<logger> simple_file_logger(const std::string& filename, const std::string& logger_name = "" )
{
    auto fsink = std::make_shared<sinks::simple_file_sink_st>(filename);
    return std::unique_ptr<logger>(new c11log::logger(logger_name, fsink));

}
std::unique_ptr<logger> simple_file_logger_mt(const std::string& filename, const std::string& logger_name = "")
{
    auto fsink = std::make_shared<sinks::simple_file_sink_mt>(filename);
    return std::unique_ptr<logger>(new c11log::logger(logger_name, fsink));
}

//
// daily file logger single/multi threaded
//
std::unique_ptr<logger> daily_file_logger(
    const std::string &filename,
    const std::string &extension,
    const std::size_t flush_every,
    const std::string& logger_name = "")
{
    auto fsink = std::make_shared<sinks::daily_file_sink_st>(filename, extension, flush_every);
    return std::unique_ptr<logger>(new c11log::logger(logger_name, fsink));
}

std::unique_ptr<logger> daily_file_logger_mt(
    const std::string &filename,
    const std::string &extension,
    const std::size_t flush_every,
    const std::string& logger_name = "")
{
    auto fsink = std::make_shared<sinks::daily_file_sink_mt>(filename, extension, flush_every);
    return std::unique_ptr<logger>(new c11log::logger(logger_name, fsink));
}

//
// rotating file logger single/multi threaded
//
std::unique_ptr<logger> rotating_file_logger(
    const std::string &filename,
    const std::string &extension,
    const std::size_t max_size,
    const std::size_t max_files,
    const std::size_t flush_every,
    const std::string& logger_name = "")
{
    auto fsink = std::make_shared<sinks::rotating_file_sink_st>(filename, extension, max_size, max_files, flush_every);
    return std::unique_ptr<logger>(new c11log::logger(logger_name, fsink));
}

std::unique_ptr<logger> rotating_file_logger_mt(
    const std::string &filename,
    const std::string &extension,
    const std::size_t max_size,
    const std::size_t max_files,
    const std::size_t flush_every,
    const std::string& logger_name = "")
{
    auto fsink = std::make_shared<sinks::rotating_file_sink_mt>(filename, extension, max_size, max_files, flush_every);
    return std::unique_ptr<logger>(new c11log::logger(logger_name, fsink));
}
} // ns factory
} // ns c11log
