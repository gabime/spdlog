// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <array>
#include <mutex>
#include <string>

#include "../details/null_mutex.h"
#include "./base_sink.h"

SPDLOG_NAMESPACE_BEGIN
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
    bool should_color() const;

    // Formatting codes
    static constexpr std::string_view reset = "\033[m";
    static constexpr std::string_view bold = "\033[1m";
    static constexpr std::string_view dark = "\033[2m";
    static constexpr std::string_view underline = "\033[4m";
    static constexpr std::string_view blink = "\033[5m";
    static constexpr std::string_view reverse = "\033[7m";
    static constexpr std::string_view concealed = "\033[8m";
    static constexpr std::string_view clear_line = "\033[K";

    // Foreground colors
    static constexpr std::string_view black = "\033[30m";
    static constexpr std::string_view red = "\033[31m";
    static constexpr std::string_view green = "\033[32m";
    static constexpr std::string_view yellow = "\033[33m";
    static constexpr std::string_view blue = "\033[34m";
    static constexpr std::string_view magenta = "\033[35m";
    static constexpr std::string_view cyan = "\033[36m";
    static constexpr std::string_view white = "\033[37m";

    // Background colors
    static constexpr std::string_view on_black = "\033[40m";
    static constexpr std::string_view on_red = "\033[41m";
    static constexpr std::string_view on_green = "\033[42m";
    static constexpr std::string_view on_yellow = "\033[43m";
    static constexpr std::string_view on_blue = "\033[44m";
    static constexpr std::string_view on_magenta = "\033[45m";
    static constexpr std::string_view on_cyan = "\033[46m";
    static constexpr std::string_view on_white = "\033[47m";

    // Bold colors
    static constexpr std::string_view yellow_bold = "\033[33m\033[1m";
    static constexpr std::string_view red_bold = "\033[31m\033[1m";
    static constexpr std::string_view bold_on_red = "\033[1m\033[41m";

protected:
    FILE *target_file_;

private:
    bool should_do_colors_;
    std::array<std::string, levels_count> colors_;

    void sink_it_(const details::log_msg &msg) override;
    void flush_() override;
    void set_color_mode_(color_mode mode);
    void print_ccode_(string_view_t color_code) const;
    void print_range_(const memory_buf_t &formatted, size_t start, size_t end) const;
    static std::string to_string_(string_view_t sv);
};

template <typename Mutex>
class ansicolor_stdout_sink final : public ansicolor_sink<Mutex> {
public:
    explicit ansicolor_stdout_sink(color_mode mode = color_mode::automatic);
};

template <typename Mutex>
class ansicolor_stderr_sink final : public ansicolor_sink<Mutex> {
public:
    explicit ansicolor_stderr_sink(color_mode mode = color_mode::automatic);
};

using ansicolor_stdout_sink_mt = ansicolor_stdout_sink<std::mutex>;
using ansicolor_stdout_sink_st = ansicolor_stdout_sink<details::null_mutex>;
using ansicolor_stderr_sink_mt = ansicolor_stderr_sink<std::mutex>;
using ansicolor_stderr_sink_st = ansicolor_stderr_sink<details::null_mutex>;

}  // namespace sinks
SPDLOG_NAMESPACE_END
