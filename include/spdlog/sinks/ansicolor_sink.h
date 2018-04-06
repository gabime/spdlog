//
// Copyright(c) 2017 spdlog authors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "../common.h"
#include "../details/os.h"
#include "base_sink.h"

#include <string>
#include <unordered_map>

namespace spdlog {
namespace sinks {

/**
 * This sink prefixes the output with an ANSI escape sequence color code depending on the severity
 * of the message.
 * If no color terminal detected, omit the escape codes.
 */
template<class Mutex>
class ansicolor_sink : public base_sink<Mutex>
{
public:
    explicit ansicolor_sink(FILE *file)
        : target_file_(file)
    {
        should_do_colors_ = details::os::in_terminal(file) && details::os::is_color_terminal();
        colors_[level::trace] = white;
        colors_[level::debug] = cyan;
        colors_[level::info] = green;
        colors_[level::warn] = yellow + bold;
        colors_[level::err] = red + bold;
        colors_[level::critical] = bold + on_red;
        colors_[level::off] = reset;
    }

    ~ansicolor_sink() override
    {
        _flush();
    }

    void set_color(level::level_enum color_level, const std::string &color)
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::_mutex);
        colors_[color_level] = color;
    }

    /// Formatting codes
    const std::string reset = "\033[m";
    const std::string bold = "\033[1m";
    const std::string dark = "\033[2m";
    const std::string underline = "\033[4m";
    const std::string blink = "\033[5m";
    const std::string reverse = "\033[7m";
    const std::string concealed = "\033[8m";
    const std::string clear_line = "\033[K";

    // Foreground colors
    const std::string black = "\033[30m";
    const std::string red = "\033[31m";
    const std::string green = "\033[32m";
    const std::string yellow = "\033[33m";
    const std::string blue = "\033[34m";
    const std::string magenta = "\033[35m";
    const std::string cyan = "\033[36m";
    const std::string white = "\033[37m";

    /// Background colors
    const std::string on_black = "\033[40m";
    const std::string on_red = "\033[41m";
    const std::string on_green = "\033[42m";
    const std::string on_yellow = "\033[43m";
    const std::string on_blue = "\033[44m";
    const std::string on_magenta = "\033[45m";
    const std::string on_cyan = "\033[46m";
    const std::string on_white = "\033[47m";

protected:
    void _sink_it(const details::log_msg &msg) override
    {
        // Wrap the originally formatted message in color codes.
        // If color is not supported in the terminal, log as is instead.
        if (should_do_colors_ && msg.color_range_end > msg.color_range_start)
        {
            // before color range
            _print_range(msg, 0, msg.color_range_start);
            // in color range
            _print_ccode(colors_[msg.level]);
            _print_range(msg, msg.color_range_start, msg.color_range_end);
            _print_ccode(reset);
            // after color range
            _print_range(msg, msg.color_range_end, msg.formatted.size());
        }
        else
        {
            _print_range(msg, 0, msg.formatted.size());
        }
        _flush();
    }

    void _flush() override
    {
        fflush(target_file_);
    }

private:
    void _print_ccode(const std::string &color_code)
    {
        fwrite(color_code.data(), sizeof(char), color_code.size(), target_file_);
    }
    void _print_range(const details::log_msg &msg, size_t start, size_t end)
    {
        fwrite(msg.formatted.data() + start, sizeof(char), end - start, target_file_);
    }
    FILE *target_file_;
    bool should_do_colors_;
    std::unordered_map<level::level_enum, std::string, level::level_hasher> colors_;
};

template<class Mutex>
class ansicolor_stdout_sink : public ansicolor_sink<Mutex>
{
public:
    ansicolor_stdout_sink()
        : ansicolor_sink<Mutex>(stdout)
    {
    }
};

using ansicolor_stdout_sink_mt = ansicolor_stdout_sink<std::mutex>;
using ansicolor_stdout_sink_st = ansicolor_stdout_sink<details::null_mutex>;

template<class Mutex>
class ansicolor_stderr_sink : public ansicolor_sink<Mutex>
{
public:
    ansicolor_stderr_sink()
        : ansicolor_sink<Mutex>(stderr)
    {
    }
};

using ansicolor_stderr_sink_mt = ansicolor_stderr_sink<std::mutex>;
using ansicolor_stderr_sink_st = ansicolor_stderr_sink<details::null_mutex>;

} // namespace sinks
} // namespace spdlog
