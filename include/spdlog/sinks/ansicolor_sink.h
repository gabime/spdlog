//
// Copyright(c) 2017 spdlog authors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#ifndef SPDLOG_H
#include "spdlog/spdlog.h"
#endif

#include "spdlog/details/console_globals.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/details/os.h"
#include "spdlog/sinks/sink.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace spdlog {
namespace sinks {

/**
 * This sink prefixes the output with an ANSI escape sequence color code
 * depending on the severity
 * of the message.
 * If no color terminal detected, omit the escape codes.
 */
template<typename TargetStream, class ConsoleMutex>
class ansicolor_sink final : public sink
{
public:
    using mutex_t = typename ConsoleMutex::mutex_t;
    ansicolor_sink()
        : target_file_(TargetStream::stream())
        , mutex_(ConsoleMutex::mutex())

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
        std::lock_guard<mutex_t> lock(mutex_);
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

    void log(const details::log_msg &msg) override
    {
        // Wrap the originally formatted message in color codes.
        // If color is not supported in the terminal, log as is instead.
        std::lock_guard<mutex_t> lock(mutex_);

        fmt::memory_buffer formatted;
        formatter_->format(msg, formatted);
        if (should_do_colors_ && msg.color_range_end > msg.color_range_start)
        {
            // before color range
            print_range_(formatted, 0, msg.color_range_start);
            // in color range
            print_ccode_(colors_[msg.level]);
            print_range_(formatted, msg.color_range_start, msg.color_range_end);
            print_ccode_(reset);
            // after color range
            print_range_(formatted, msg.color_range_end, formatted.size());
        }
        else // no color
        {
            print_range_(formatted, 0, formatted.size());
        }
        fflush(target_file_);
    }

    void flush() override
    {
        std::lock_guard<mutex_t> lock(mutex_);
        fflush(target_file_);
    }

    void set_pattern(const std::string &pattern) final
    {
        std::lock_guard<mutex_t> lock(mutex_);
        formatter_ = std::unique_ptr<spdlog::formatter>(new pattern_formatter(pattern));
    }

    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override
    {
        std::lock_guard<mutex_t> lock(mutex_);
        formatter_ = std::move(sink_formatter);
    }

private:
    void print_ccode_(const std::string &color_code)
    {
        fwrite(color_code.data(), sizeof(char), color_code.size(), target_file_);
    }
    void print_range_(const fmt::memory_buffer &formatted, size_t start, size_t end)
    {
        fwrite(formatted.data() + start, sizeof(char), end - start, target_file_);
    }

    FILE *target_file_;
    mutex_t &mutex_;

    bool should_do_colors_;
    std::unordered_map<level::level_enum, std::string, level::level_hasher> colors_;
};

using ansicolor_stdout_sink_mt = ansicolor_sink<details::console_stdout, details::console_mutex>;
using ansicolor_stdout_sink_st = ansicolor_sink<details::console_stdout, details::console_nullmutex>;

using ansicolor_stderr_sink_mt = ansicolor_sink<details::console_stderr, details::console_mutex>;
using ansicolor_stderr_sink_st = ansicolor_sink<details::console_stderr, details::console_nullmutex>;

} // namespace sinks

} // namespace spdlog
