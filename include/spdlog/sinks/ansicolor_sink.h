// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/details/color_helper.h>
#include <spdlog/details/console_globals.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/sink.h>
#include <memory>
#include <mutex>
#include <string>
#include <array>

namespace spdlog {
namespace sinks {

/**
 * This sink prefixes the output with an ANSI escape sequence color code
 * depending on the severity
 * of the message.
 * If no color terminal detected, omit the escape codes.
 */

template<typename ConsoleMutex>
class ansicolor_sink : public sink
{
public:
    using mutex_t = typename ConsoleMutex::mutex_t;
    ansicolor_sink(FILE *target_file, color_mode mode);
    ~ansicolor_sink() override = default;

    ansicolor_sink(const ansicolor_sink &other) = delete;
    ansicolor_sink(ansicolor_sink &&other) = delete;

    ansicolor_sink &operator=(const ansicolor_sink &other) = delete;
    ansicolor_sink &operator=(ansicolor_sink &&other) = delete;

    void set_style(level::level_enum color_level, spdlog::style style);
    void set_color_mode(color_mode mode);
    bool should_color();

    void log(const details::log_msg &msg) override;
    void flush() override;
    void set_pattern(const std::string &pattern) final;
    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override;

private:
    FILE *target_file_;
    mutex_t &mutex_;
    bool should_do_colors_;
    std::unique_ptr<spdlog::formatter> formatter_;
    std::array<std::string, level::n_levels> colors_;
    void print_ccode_(const string_view_t &color_code);
    void print_range_(const memory_buf_t &formatted, size_t start, size_t end);
    static std::string to_string_(const string_view_t &sv);
};

template<typename ConsoleMutex>
class ansicolor_stdout_sink : public ansicolor_sink<ConsoleMutex>
{
public:
    explicit ansicolor_stdout_sink(color_mode mode = color_mode::automatic);
};

template<typename ConsoleMutex>
class ansicolor_stderr_sink : public ansicolor_sink<ConsoleMutex>
{
public:
    explicit ansicolor_stderr_sink(color_mode mode = color_mode::automatic);
};

using ansicolor_stdout_sink_mt = ansicolor_stdout_sink<details::console_mutex>;
using ansicolor_stdout_sink_st = ansicolor_stdout_sink<details::console_nullmutex>;

using ansicolor_stderr_sink_mt = ansicolor_stderr_sink<details::console_mutex>;
using ansicolor_stderr_sink_st = ansicolor_stderr_sink<details::console_nullmutex>;

} // namespace sinks
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#    include "ansicolor_sink-inl.h"
#endif
