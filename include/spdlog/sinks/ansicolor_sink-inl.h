// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include "spdlog/sinks/ansicolor_sink.h"
#endif

#include "spdlog/details/os.h"

template<typename TargetStream, typename ConsoleMutex>
SPDLOG_INLINE spdlog::sinks::ansicolor_sink<TargetStream, ConsoleMutex>::ansicolor_sink(color_mode mode)
    : target_file_(TargetStream::stream())
    , mutex_(ConsoleMutex::mutex())

{
    set_color_mode(mode);
    colors_[level::trace] = white;
    colors_[level::debug] = cyan;
    colors_[level::info] = green;
    colors_[level::warn] = yellow + bold;
    colors_[level::err] = red + bold;
    colors_[level::critical] = bold + on_red;
    colors_[level::off] = reset;
}

template<typename TargetStream, typename ConsoleMutex>
SPDLOG_INLINE void spdlog::sinks::ansicolor_sink<TargetStream, ConsoleMutex>::set_color(
    level::level_enum color_level, const std::string &color)
{
    std::lock_guard<mutex_t> lock(mutex_);
    colors_[color_level] = color;
}

template<typename TargetStream, typename ConsoleMutex>
SPDLOG_INLINE void spdlog::sinks::ansicolor_sink<TargetStream, ConsoleMutex>::log(const details::log_msg &msg)
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

template<typename TargetStream, typename ConsoleMutex>
SPDLOG_INLINE void spdlog::sinks::ansicolor_sink<TargetStream, ConsoleMutex>::flush()
{
    std::lock_guard<mutex_t> lock(mutex_);
    fflush(target_file_);
}

template<typename TargetStream, typename ConsoleMutex>
SPDLOG_INLINE void spdlog::sinks::ansicolor_sink<TargetStream, ConsoleMutex>::set_pattern(const std::string &pattern)
{
    std::lock_guard<mutex_t> lock(mutex_);
    formatter_ = std::unique_ptr<spdlog::formatter>(new pattern_formatter(pattern));
}

template<typename TargetStream, typename ConsoleMutex>
SPDLOG_INLINE void spdlog::sinks::ansicolor_sink<TargetStream, ConsoleMutex>::set_formatter(
    std::unique_ptr<spdlog::formatter> sink_formatter)
{
    std::lock_guard<mutex_t> lock(mutex_);
    formatter_ = std::move(sink_formatter);
}

template<typename TargetStream, typename ConsoleMutex>
SPDLOG_INLINE bool spdlog::sinks::ansicolor_sink<TargetStream, ConsoleMutex>::should_color()
{
    return should_do_colors_;
}

template<typename TargetStream, typename ConsoleMutex>
SPDLOG_INLINE void spdlog::sinks::ansicolor_sink<TargetStream, ConsoleMutex>::set_color_mode(color_mode mode)
{
    switch (mode)
    {
    case color_mode::always:
        should_do_colors_ = true;
        return;
    case color_mode::automatic:
        should_do_colors_ = details::os::in_terminal(target_file_) && details::os::is_color_terminal();
        return;
    case color_mode::never:
        should_do_colors_ = false;
        return;
    }
}

template<typename TargetStream, typename ConsoleMutex>
SPDLOG_INLINE void spdlog::sinks::ansicolor_sink<TargetStream, ConsoleMutex>::print_ccode_(const std::string &color_code)
{
    fwrite(color_code.data(), sizeof(char), color_code.size(), target_file_);
}

template<typename TargetStream, typename ConsoleMutex>
SPDLOG_INLINE void spdlog::sinks::ansicolor_sink<TargetStream, ConsoleMutex>::print_range_(
    const fmt::memory_buffer &formatted, size_t start, size_t end)
{
    fwrite(formatted.data() + start, sizeof(char), end - start, target_file_);
}
