//
// Copyright(c) 2017 spdlog authors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "spdlog/sinks/base_sink.h"
#include "spdlog/common.h"
#include "spdlog/details/os.h"

#include <string>
#include <map>

namespace spdlog
{
namespace sinks
{

/**
 * This sink prefixes the output with an ANSI escape sequence color code depending on the severity
 * of the message. 
 * If no color terminal detected, omit the escape codes.
 */
template <class Mutex>
class ansicolor_sink: public base_sink<Mutex>
{
public:
    ansicolor_sink(FILE* file): target_file_(file)
    {
        should_do_colors_ = details::os::in_terminal(file) && details::os::is_color_terminal();
        colors_[level::trace] = "\033[36m"; // cyan;
        colors_[level::debug] = "\033[36m"; // cyan;
        colors_[level::info] = "\033[0m";// default color;
        colors_[level::warn] = "\033[33m\033[1m"; // yellow_bold;
        colors_[level::err] = "\033[31m\033[1m"; // red_bold;
        colors_[level::critical] = "\033[1m\033[41m"; // bold_red_bg;
        colors_[level::off] = "\033[0m"; //reset to default;
    }
    virtual ~ansicolor_sink()
    {
        _flush();
    }

    

protected:
    virtual void _sink_it(const details::log_msg& msg) override
    {
        // Wrap the originally formatted message in color codes.
        // If color is not supported in the terminal, log as is instead.
        if (should_do_colors_)
        {
            const std::string& prefix = colors_[msg.level];
            const std::string& reset = colors_[level::off];
            fwrite(prefix.data(), sizeof(char), prefix.size(), target_file_);
            fwrite(msg.formatted.data(), sizeof(char), msg.formatted.size(), target_file_);
            fwrite(reset.data(), sizeof(char), reset.size(), target_file_);
        }
        else
        {
            fwrite(msg.formatted.data(), sizeof(char), msg.formatted.size(), target_file_);
        }
        _flush();
    }

	void _flush() override
	{
		fflush(target_file_);
	}
    FILE* target_file_;
    bool should_do_colors_;
    std::map<level::level_enum, std::string> colors_;
};


template<class Mutex>
class ansicolor_stdout_sink: public ansicolor_sink<Mutex>
{
public:
    ansicolor_stdout_sink(): ansicolor_sink<Mutex>(stdout)
    {}
};

template<class Mutex>
class ansicolor_stderr_sink: public ansicolor_sink<Mutex>
{
public:
    ansicolor_stderr_sink(): ansicolor_sink<Mutex>(stderr)
    {}
};

typedef ansicolor_stdout_sink<std::mutex> ansicolor_stdout_sink_mt;
typedef ansicolor_stdout_sink<details::null_mutex> ansicolor_stdout_sink_st;

typedef ansicolor_stderr_sink<std::mutex> ansicolor_stderr_sink_mt;
typedef ansicolor_stderr_sink<details::null_mutex> ansicolor_stderr_sink_st;

} // namespace sinks
} // namespace spdlog

