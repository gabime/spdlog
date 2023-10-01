// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <array>
#include <memory>
#include <mutex>
#include <string>

#include "../details/null_mutex.h"
#include "base_sink.h"
#include "sink.h"

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

private:
    void sink_it_(const details::log_msg &msg) override;
    void flush_() override;
    FILE *target_file_;
    bool should_do_colors_;
    std::array<std::string, levels_count> colors_;
    void print_ccode_(const string_view_t &color_code);
    void print_range_(const memory_buf_t &formatted, size_t start, size_t end);
    static std::string to_string_(const string_view_t &sv);
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
