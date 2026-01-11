// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <array>
#include <memory>
#include <mutex>
#include <spdlog/details/console_globals.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/sink.h>
#include <string>

namespace spdlog {
namespace sinks {

/**
 * This sink prefixes the output with an ANSI escape sequence color code
 * depending on the severity
 * of the message.
 * If no color terminal detected, omit the escape codes.
 */

template <typename ConsoleMutex>
class ansicolor_sink : public sink {
public:
    using mutex_t = typename ConsoleMutex::mutex_t;
    ansicolor_sink(FILE* target_file, color_mode mode);
    ~ansicolor_sink() override = default;

    ansicolor_sink(const ansicolor_sink& other) = delete;
    ansicolor_sink(ansicolor_sink&& other) = delete;

    ansicolor_sink& operator=(const ansicolor_sink& other) = delete;
    ansicolor_sink& operator=(ansicolor_sink&& other) = delete;

    void set_color(level::level_enum color_level, string_view_t color);
    void set_color_mode(color_mode mode);
    bool should_color() const;

    void log(const details::log_msg& msg) override;
    void flush() override;
    void set_pattern(const std::string& pattern) override;
    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override;

    // Formatting codes
    static constexpr const char* reset = "\033[m";
    static constexpr const char* bold = "\033[1m";
    static constexpr const char* dark = "\033[2m";
    static constexpr const char* underline = "\033[4m";
    static constexpr const char* blink = "\033[5m";
    static constexpr const char* reverse = "\033[7m";
    static constexpr const char* concealed = "\033[8m";
    static constexpr const char* clear_line = "\033[K";

    // Foreground colors
    static constexpr const char* black = "\033[30m";
    static constexpr const char* red = "\033[31m";
    static constexpr const char* green = "\033[32m";
    static constexpr const char* yellow = "\033[33m";
    static constexpr const char* blue = "\033[34m";
    static constexpr const char* magenta = "\033[35m";
    static constexpr const char* cyan = "\033[36m";
    static constexpr const char* white = "\033[37m";

    /// Background colors
    static constexpr const char* on_black = "\033[40m";
    static constexpr const char* on_red = "\033[41m";
    static constexpr const char* on_green = "\033[42m";
    static constexpr const char* on_yellow = "\033[43m";
    static constexpr const char* on_blue = "\033[44m";
    static constexpr const char* on_magenta = "\033[45m";
    static constexpr const char* on_cyan = "\033[46m";
    static constexpr const char* on_white = "\033[47m";

    /// Bold colors
    static constexpr const char* yellow_bold = "\033[33m\033[1m";
    static constexpr const char* red_bold = "\033[31m\033[1m";
    static constexpr const char* bold_on_red = "\033[1m\033[41m";

protected:
    FILE* target_file_;

private:
    mutex_t& mutex_;
    bool should_do_colors_;
    std::unique_ptr<spdlog::formatter> formatter_;
    std::array<std::string, level::n_levels> colors_{
        white,        // TRACE
        cyan,         // DEBUG
        green,        // INFO
        yellow_bold,  // WARN
        red_bold,     // ERROR
        bold_on_red,  // CRITICAL
        reset         // OFF
    };
    void set_color_mode_(color_mode mode);
    void print_ccode_(const string_view_t& color_code) const;
    void print_range_(const memory_buf_t& formatted, size_t start, size_t end) const;
    static std::string to_string_(const string_view_t& sv);
};

template <typename ConsoleMutex>
class ansicolor_stdout_sink : public ansicolor_sink<ConsoleMutex> {
public:
    explicit ansicolor_stdout_sink(color_mode mode = color_mode::automatic);
};

template <typename ConsoleMutex>
class ansicolor_stderr_sink : public ansicolor_sink<ConsoleMutex> {
public:
    explicit ansicolor_stderr_sink(color_mode mode = color_mode::automatic);
};

using ansicolor_stdout_sink_mt = ansicolor_stdout_sink<details::console_mutex>;
using ansicolor_stdout_sink_st = ansicolor_stdout_sink<details::console_nullmutex>;

using ansicolor_stderr_sink_mt = ansicolor_stderr_sink<details::console_mutex>;
using ansicolor_stderr_sink_st = ansicolor_stderr_sink<details::console_nullmutex>;

}  // namespace sinks
}  // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "ansicolor_sink-inl.h"
#endif
