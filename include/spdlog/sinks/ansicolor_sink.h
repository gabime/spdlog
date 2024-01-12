// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <array>
#include <memory>
#include <mutex>
#include <string>

#include "../details/null_mutex.h"
#include "./base_sink.h"

namespace spdlog {
namespace sinks {

/**
 * This sink prefixes the output with an ANSI escape sequence color code
 * depending on the severity
 * of the message.
 * If no color terminal detected, omit the escape codes.
 */

template <typename Mutex>
class ansicolor_sink : public base_sink<Mutex> {
public:
    ansicolor_sink(FILE *target_file, color_mode mode);
    ansicolor_sink(const ansicolor_sink &other) = delete;
    ansicolor_sink(ansicolor_sink &&other) = delete;
    ansicolor_sink &operator=(const ansicolor_sink &other) = delete;
    ansicolor_sink &operator=(ansicolor_sink &&other) = delete;
    ~ansicolor_sink() override = default;

    void set_color(level color_level, string_view_t color);
    void set_color_mode(color_mode mode);
    bool should_color();

    // Formatting codes
    static constexpr spdlog::string_view_t reset = "\033[m";
    static constexpr spdlog::string_view_t bold = "\033[1m";
    static constexpr spdlog::string_view_t dark = "\033[2m";
    static constexpr spdlog::string_view_t underline = "\033[4m";
    static constexpr spdlog::string_view_t blink = "\033[5m";
    static constexpr spdlog::string_view_t reverse = "\033[7m";
    static constexpr spdlog::string_view_t concealed = "\033[8m";
    static constexpr spdlog::string_view_t clear_line = "\033[K";

    // Foreground colors
    static constexpr spdlog::string_view_t black = "\033[30m";
    static constexpr spdlog::string_view_t red = "\033[31m";
    static constexpr spdlog::string_view_t green = "\033[32m";
    static constexpr spdlog::string_view_t yellow = "\033[33m";
    static constexpr spdlog::string_view_t blue = "\033[34m";
    static constexpr spdlog::string_view_t magenta = "\033[35m";
    static constexpr spdlog::string_view_t cyan = "\033[36m";
    static constexpr spdlog::string_view_t white = "\033[37m";

    // Background colors
    static constexpr spdlog::string_view_t on_black = "\033[40m";
    static constexpr spdlog::string_view_t on_red = "\033[41m";
    static constexpr spdlog::string_view_t on_green = "\033[42m";
    static constexpr spdlog::string_view_t on_yellow = "\033[43m";
    static constexpr spdlog::string_view_t on_blue = "\033[44m";
    static constexpr spdlog::string_view_t on_magenta = "\033[45m";
    static constexpr spdlog::string_view_t on_cyan = "\033[46m";
    static constexpr spdlog::string_view_t on_white = "\033[47m";

    // Bold colors
    static constexpr spdlog::string_view_t yellow_bold = "\033[33m\033[1m";
    static constexpr spdlog::string_view_t red_bold = "\033[31m\033[1m";
    static constexpr spdlog::string_view_t bold_on_red = "\033[1m\033[41m";

private:
    void sink_it_(const details::log_msg &msg) override;
    void flush_() override;
    FILE *target_file_;
    bool should_do_colors_;
    std::array<std::string, levels_count> colors_;
    void print_ccode_(const string_view_t color_code);
    void print_range_(const memory_buf_t &formatted, size_t start, size_t end);
    static std::string to_string_(const string_view_t sv);
};

template <typename Mutex>
class ansicolor_stdout_sink : public ansicolor_sink<Mutex> {
public:
    explicit ansicolor_stdout_sink(color_mode mode = color_mode::automatic);
};

template <typename Mutex>
class ansicolor_stderr_sink : public ansicolor_sink<Mutex> {
public:
    explicit ansicolor_stderr_sink(color_mode mode = color_mode::automatic);
};

using ansicolor_stdout_sink_mt = ansicolor_stdout_sink<std::mutex>;
using ansicolor_stdout_sink_st = ansicolor_stdout_sink<details::null_mutex>;

using ansicolor_stderr_sink_mt = ansicolor_stderr_sink<std::mutex>;
using ansicolor_stderr_sink_st = ansicolor_stderr_sink<details::null_mutex>;

}  // namespace sinks
}  // namespace spdlog
