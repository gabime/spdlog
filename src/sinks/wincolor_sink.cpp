// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// clang-format off
#include "spdlog/details/windows_include.h"
#include <wincon.h>
#include <mutex>
// clang-format on

#include "spdlog/sinks/wincolor_sink.h"

#include "spdlog/common.h"
#include "spdlog/details/null_mutex.h"

namespace spdlog {
namespace sinks {
template <typename Mutex>
wincolor_sink<Mutex>::wincolor_sink(void *out_handle, color_mode mode)
    : out_handle_(out_handle) {
    set_color_mode_impl(mode);
    // set level colors
    colors_.at(level_to_number(level::trace)) = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;      // white
    colors_.at(level_to_number(level::debug)) = FOREGROUND_GREEN | FOREGROUND_BLUE;                       // cyan
    colors_.at(level_to_number(level::info)) = FOREGROUND_GREEN;                                          // green
    colors_.at(level_to_number(level::warn)) = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;  // intense yellow
    colors_.at(level_to_number(level::err)) = FOREGROUND_RED | FOREGROUND_INTENSITY;                      // intense red
    colors_.at(level_to_number(level::critical)) = BACKGROUND_RED | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE |
                                                   FOREGROUND_INTENSITY;  // intense white on red background
    colors_.at(level_to_number(level::off)) = 0;
}

template <typename Mutex>
wincolor_sink<Mutex>::~wincolor_sink() {
    this->flush();
}

// change the color for the given level
template <typename Mutex>
void wincolor_sink<Mutex>::set_color(level level, std::uint16_t color) {
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    colors_[level_to_number(level)] = color;
}

template <typename Mutex>
void wincolor_sink<Mutex>::sink_it_(const details::log_msg &msg) {
    if (out_handle_ == nullptr || out_handle_ == INVALID_HANDLE_VALUE) {
        return;
    }

    msg.color_range_start = 0;
    msg.color_range_end = 0;
    memory_buf_t formatted;
    base_sink<Mutex>::formatter_->format(msg, formatted);
    if (should_do_colors_ && msg.color_range_end > msg.color_range_start) {
        // before color range
        print_range_(formatted, 0, msg.color_range_start);
        // in color range
        auto orig_attribs = static_cast<WORD>(set_foreground_color_(colors_[static_cast<size_t>(msg.log_level)]));
        print_range_(formatted, msg.color_range_start, msg.color_range_end);
        // reset to orig colors
        ::SetConsoleTextAttribute(static_cast<HANDLE>(out_handle_), orig_attribs);
        print_range_(formatted, msg.color_range_end, formatted.size());
    } else  // print without colors if color range is invalid (or color is disabled)
    {
        write_to_file_(formatted);
    }
}

template <typename Mutex>
void wincolor_sink<Mutex>::flush_() {
    // windows console always flushed?
}

template <typename Mutex>
void wincolor_sink<Mutex>::set_color_mode(color_mode mode) {
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    set_color_mode_impl(mode);
}

template <typename Mutex>
void wincolor_sink<Mutex>::set_color_mode_impl(color_mode mode) {
    if (mode == color_mode::automatic) {
        // should do colors only if out_handle_  points to actual console.
        DWORD console_mode;
        bool in_console = ::GetConsoleMode(static_cast<HANDLE>(out_handle_), &console_mode) != 0;
        should_do_colors_ = in_console;
    } else {
        should_do_colors_ = mode == color_mode::always ? true : false;
    }
}

// set foreground color and return the orig console attributes (for resetting later)
template <typename Mutex>
std::uint16_t wincolor_sink<Mutex>::set_foreground_color_(std::uint16_t attribs) {
    CONSOLE_SCREEN_BUFFER_INFO orig_buffer_info;
    if (!::GetConsoleScreenBufferInfo(static_cast<HANDLE>(out_handle_), &orig_buffer_info)) {
        // just return white if failed getting console info
        return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }

    // change only the foreground bits (lowest 4 bits)
    auto new_attribs = static_cast<WORD>(attribs) | (orig_buffer_info.wAttributes & 0xfff0);
    auto ignored = ::SetConsoleTextAttribute(static_cast<HANDLE>(out_handle_), static_cast<WORD>(new_attribs));
    (void)(ignored);
    return static_cast<std::uint16_t>(orig_buffer_info.wAttributes);  // return orig attribs
}

// print a range of formatted message to console
template <typename Mutex>
void wincolor_sink<Mutex>::print_range_(const memory_buf_t &formatted, size_t start, size_t end) {
    if (end > start) {
        auto size = static_cast<DWORD>(end - start);
        auto ignored = ::WriteConsoleA(static_cast<HANDLE>(out_handle_), formatted.data() + start, size, nullptr, nullptr);
        (void)(ignored);
    }
}

template <typename Mutex>
void wincolor_sink<Mutex>::write_to_file_(const memory_buf_t &formatted) {
    auto size = static_cast<DWORD>(formatted.size());
    DWORD bytes_written = 0;
    auto ignored = ::WriteFile(static_cast<HANDLE>(out_handle_), formatted.data(), size, &bytes_written, nullptr);
    (void)(ignored);
}

// wincolor_stdout_sink
template <typename Mutex>
wincolor_stdout_sink<Mutex>::wincolor_stdout_sink(color_mode mode)
    : wincolor_sink<Mutex>(::GetStdHandle(STD_OUTPUT_HANDLE), mode) {}

// wincolor_stderr_sink
template <typename Mutex>
wincolor_stderr_sink<Mutex>::wincolor_stderr_sink(color_mode mode)
    : wincolor_sink<Mutex>(::GetStdHandle(STD_ERROR_HANDLE), mode) {}
}  // namespace sinks
}  // namespace spdlog

// template instantiations
template class SPDLOG_API spdlog::sinks::wincolor_sink<std::mutex>;
template class SPDLOG_API spdlog::sinks::wincolor_sink<spdlog::details::null_mutex>;

template class SPDLOG_API spdlog::sinks::wincolor_stdout_sink<std::mutex>;
template class SPDLOG_API spdlog::sinks::wincolor_stdout_sink<spdlog::details::null_mutex>;

template class SPDLOG_API spdlog::sinks::wincolor_stderr_sink<std::mutex>;
template class SPDLOG_API spdlog::sinks::wincolor_stderr_sink<spdlog::details::null_mutex>;
