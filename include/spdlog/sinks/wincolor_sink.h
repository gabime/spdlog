// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/common.h"
#include "spdlog/details/console_globals.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/sinks/sink.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <wincon.h>

namespace spdlog {
namespace sinks {
/*
 * Windows color console sink. Uses WriteConsoleA to write to the console with
 * colors
 */
template<typename TargetStream, typename ConsoleMutex>
class wincolor_sink : public sink
{
public:
    const WORD BOLD = FOREGROUND_INTENSITY;
    const WORD RED = FOREGROUND_RED;
    const WORD GREEN = FOREGROUND_GREEN;
    const WORD CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE;
    const WORD WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    const WORD YELLOW = FOREGROUND_RED | FOREGROUND_GREEN;

    wincolor_sink(color_mode mode = color_mode::automatic);
    ~wincolor_sink() override;

    wincolor_sink(const wincolor_sink &other) = delete;
    wincolor_sink &operator=(const wincolor_sink &other) = delete;

    // change the color for the given level
    void set_color(level::level_enum level, WORD color);
    void log(const details::log_msg &msg) final override;
    void flush() final override;
    void set_pattern(const std::string &pattern) override final;
    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override final;
    void set_color_mode(color_mode mode);

private:
    using mutex_t = typename ConsoleMutex::mutex_t;
    HANDLE out_handle_;
    mutex_t &mutex_;
    bool should_do_colors_;
    std::unordered_map<level::level_enum, WORD, level::level_hasher> colors_;

    // set color and return the orig console attributes (for resetting later)
    WORD set_console_attribs(WORD attribs);
    // print a range of formatted message to console
    void print_range_(const fmt::memory_buffer &formatted, size_t start, size_t end);
};

using wincolor_stdout_sink_mt = wincolor_sink<details::console_stdout, details::console_mutex>;
using wincolor_stdout_sink_st = wincolor_sink<details::console_stdout, details::console_nullmutex>;

using wincolor_stderr_sink_mt = wincolor_sink<details::console_stderr, details::console_mutex>;
using wincolor_stderr_sink_st = wincolor_sink<details::console_stderr, details::console_nullmutex>;

} // namespace sinks
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "wincolor_sink-inl.h"
#endif
