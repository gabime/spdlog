// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/details/console_globals.h"
#include "spdlog/details/null_mutex.h"
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
template<typename TargetStream, typename ConsoleMutex>
class ansicolor_sink final : public sink
{
public:
    using mutex_t = typename ConsoleMutex::mutex_t;
    explicit ansicolor_sink(color_mode mode = color_mode::automatic);
    ~ansicolor_sink() override = default;

    ansicolor_sink(const ansicolor_sink &other) = delete;
    ansicolor_sink &operator=(const ansicolor_sink &other) = delete;
    void set_color(level::level_enum color_level, const std::string &color);
    void log(const details::log_msg &msg) override;
    void flush() override;
    void set_pattern(const std::string &pattern) final;
    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override;
    bool should_color();
    void set_color_mode(color_mode mode);

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

private:
    FILE *target_file_;
    mutex_t &mutex_;
    bool should_do_colors_;
    std::unordered_map<level::level_enum, std::string, level::level_hasher> colors_;
    void print_ccode_(const std::string &color_code);
    void print_range_(const fmt::memory_buffer &formatted, size_t start, size_t end);
};

using ansicolor_stdout_sink_mt = ansicolor_sink<details::console_stdout, details::console_mutex>;
using ansicolor_stdout_sink_st = ansicolor_sink<details::console_stdout, details::console_nullmutex>;

using ansicolor_stderr_sink_mt = ansicolor_sink<details::console_stderr, details::console_mutex>;
using ansicolor_stderr_sink_st = ansicolor_sink<details::console_stderr, details::console_nullmutex>;

} // namespace sinks
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "ansicolor_sink-inl.h"
#endif
