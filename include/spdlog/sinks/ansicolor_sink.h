//
// Copyright(c) 2017 spdlog authors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "../details/null_mutex.h"
#include "../details/os.h"
#include "../details/traits.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace spdlog {
namespace sinks {

/**
 * This sink prefixes the output with an ANSI escape sequence color code depending on the severity
 * of the message.
 * If no color terminal detected, omit the escape codes.
 */
template<class ConsoleStdoutTrait, class ConsoleMutexTrait>
class ansicolor_sink : public sink
{
public:
    using mutex_t = typename ConsoleMutexTrait::mutex_t;
    ansicolor_sink()
        : target_file_(ConsoleStdoutTrait::stream())
        , _mutex(ConsoleMutexTrait::console_mutex())

    {
        should_do_colors_ = details::os::in_terminal(target_file_) && details::os::is_color_terminal();
        colors_[level::trace] = white;
        colors_[level::debug] = cyan;
        colors_[level::info] = green;
        colors_[level::warn] = yellow + bold;
        colors_[level::err] = red + bold;
        colors_[level::critical] = bold + on_red;
        colors_[level::off] = reset;
    }

    ~ansicolor_sink() override = default;

    ansicolor_sink(const ansicolor_sink &other) = delete;
    ansicolor_sink &operator=(const ansicolor_sink &other) = delete;

    void set_color(level::level_enum color_level, const std::string &color)
    {
        std::lock_guard<mutex_t> lock(_mutex);
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

    void log(const details::log_msg &msg) SPDLOG_FINAL override
    {
        // Wrap the originally formatted message in color codes.
        // If color is not supported in the terminal, log as is instead.
        std::lock_guard<mutex_t> lock(_mutex);
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
        else // no color
        {
            _print_range(msg, 0, msg.formatted.size());
        }
        fflush(target_file_);
    }

    void flush() SPDLOG_FINAL override
    {
        std::lock_guard<mutex_t> lock(_mutex);
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
    mutex_t &_mutex;

    bool should_do_colors_;
    std::unordered_map<level::level_enum, std::string, level::level_hasher> colors_;
};

using ansicolor_stdout_sink_mt = ansicolor_sink<details::console_stdout_trait, details::console_mutex_trait>;
using ansicolor_stdout_sink_st = ansicolor_sink<details::console_stdout_trait, details::console_null_mutex_trait>;
using ansicolor_stderr_sink_mt = ansicolor_sink<details::console_stderr_trait, details::console_mutex_trait>;
using ansicolor_stderr_sink_st = ansicolor_sink<details::console_stderr_trait, details::console_null_mutex_trait>;

} // namespace sinks

} // namespace spdlog
