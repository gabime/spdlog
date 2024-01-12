// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include "../common.h"
#include "../details/null_mutex.h"
#include "./base_sink.h"

namespace spdlog {
namespace sinks {
/*
 * Windows color console sink. Uses WriteConsoleA to write to the console with
 * colors
 */
template <typename Mutex>
class wincolor_sink : public base_sink<Mutex> {
public:
    wincolor_sink(void *out_handle, color_mode mode);
    ~wincolor_sink() override;

    wincolor_sink(const wincolor_sink &other) = delete;
    wincolor_sink &operator=(const wincolor_sink &other) = delete;

    // change the color for the given level
    void set_color(level level, std::uint16_t color);
    void set_color_mode(color_mode mode);

private:
    void *out_handle_;
    bool should_do_colors_;
    std::array<std::uint16_t, levels_count> colors_;

    // set foreground color and return the orig console attributes (for resetting later)
    std::uint16_t set_foreground_color_(std::uint16_t attribs);

    void sink_it_(const details::log_msg &msg) override;
    void flush_() override;

    // print a range of formatted message to console
    void print_range_(const memory_buf_t &formatted, size_t start, size_t end);

    // in case we are redirected to file (not in console mode)
    void write_to_file_(const memory_buf_t &formatted);

    void set_color_mode_impl(color_mode mode);
};

template <typename Mutex>
class wincolor_stdout_sink : public wincolor_sink<Mutex> {
public:
    explicit wincolor_stdout_sink(color_mode mode = color_mode::automatic);
};

template <typename Mutex>
class wincolor_stderr_sink : public wincolor_sink<Mutex> {
public:
    explicit wincolor_stderr_sink(color_mode mode = color_mode::automatic);
};

using wincolor_stdout_sink_mt = wincolor_stdout_sink<std::mutex>;
using wincolor_stdout_sink_st = wincolor_stdout_sink<details::null_mutex>;

using wincolor_stderr_sink_mt = wincolor_stderr_sink<std::mutex>;
using wincolor_stderr_sink_st = wincolor_stderr_sink<details::null_mutex>;
}  // namespace sinks
}  // namespace spdlog
