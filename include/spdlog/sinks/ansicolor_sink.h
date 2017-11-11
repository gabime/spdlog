//
// Copyright(c) 2017 spdlog authors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "base_sink.h"
#include "../common.h"
#include "../details/os.h"

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
        colors_[level::trace] = cyan;
        colors_[level::debug] = cyan;
        colors_[level::info] = reset;
        colors_[level::warn] = yellow + bold;
        colors_[level::err] = red + bold;
        colors_[level::critical] = bold + on_red;
        colors_[level::off] = reset;
    }
    virtual ~ansicolor_sink()
    {
        _flush();
    }

    void set_color(level::level_enum color_level, const std::string& color)
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::_mutex);
        colors_[color_level] = color;
    }

    /// Formatting codes
    const std::string reset = "\033[00m";
    const std::string bold = "\033[1m";
    const std::string dark = "\033[2m";
    const std::string underline = "\033[4m";
    const std::string blink = "\033[5m";
    const std::string reverse = "\033[7m";
    const std::string concealed = "\033[8m";

    // Foreground colors
    const std::string grey = "\033[30m";
    const std::string red = "\033[31m";
    const std::string green = "\033[32m";
    const std::string yellow = "\033[33m";
    const std::string blue = "\033[34m";
    const std::string magenta = "\033[35m";
    const std::string cyan = "\033[36m";
    const std::string white = "\033[37m";

    /// Background colors
    const std::string on_grey = "\033[40m";
    const std::string on_red = "\033[41m";
    const std::string on_green = "\033[42m";
    const std::string on_yellow = "\033[43m";
    const std::string on_blue = "\033[44m";
    const std::string on_magenta = "\033[45m";
    const std::string on_cyan = "\033[46m";
    const std::string on_white = "\033[47m";

protected:
    virtual void _sink_it(const details::log_msg& msg) override
    {
        // Wrap the originally formatted message in color codes.
        // If color is not supported in the terminal, log as is instead.
        if (should_do_colors_)
        {
            const std::string& prefix = colors_[msg.level];
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

