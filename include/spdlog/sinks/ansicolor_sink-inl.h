// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#    include <spdlog/sinks/ansicolor_sink.h>
#endif

#include <spdlog/pattern_formatter.h>
#include <spdlog/details/os.h>

namespace spdlog {
namespace sinks {

template<typename ConsoleMutex>
SPDLOG_INLINE ansicolor_sink<ConsoleMutex>::ansicolor_sink(FILE *target_file, color_mode mode)
    : target_file_(target_file)
    , mutex_(ConsoleMutex::mutex())
    , formatter_(details::make_unique<spdlog::pattern_formatter>())

{
    set_color_mode(mode);
#if !defined(_WIN32) && defined(SPDLOG_EXTENDED_STLYING)
    colors_.at(level::trace)    = details::styles_array{details::style_type::fg_white};
    colors_.at(level::debug)    = details::styles_array{details::style_type::fg_cyan};
    colors_.at(level::info)     = details::styles_array{details::style_type::fg_green};
    colors_.at(level::warn)     = details::styles_array{details::style_type::bold,details::style_type::fg_yellow};
    colors_.at(level::err)      = details::styles_array{details::style_type::bold,details::style_type::fg_red};
    colors_.at(level::critical) = details::styles_array{details::style_type::bold,details::style_type::bg_red};
    colors_.at(level::off)      = details::styles_array{details::style_type::reset};
#else
    colors_.at(level::trace) = to_string_(white);
    colors_.at(level::debug) = to_string_(cyan);
    colors_.at(level::info) = to_string_(green);
    colors_.at(level::warn) = to_string_(yellow_bold);
    colors_.at(level::err) = to_string_(red_bold);
    colors_.at(level::critical) = to_string_(bold_on_red);
    colors_.at(level::off) = to_string_(reset);
#endif
}

#if !defined(_WIN32) && defined(SPDLOG_EXTENDED_STLYING)
template<typename ConsoleMutex>
SPDLOG_INLINE void ansicolor_sink<ConsoleMutex>::set_color(level::level_enum color_level, details::styles_array color)
{
    std::lock_guard<mutex_t> lock(mutex_);
    colors_.at(static_cast<size_t>(color_level)) = color;
}
#else
template<typename ConsoleMutex>
SPDLOG_INLINE void ansicolor_sink<ConsoleMutex>::set_color(level::level_enum color_level, string_view_t color)
{
    std::lock_guard<mutex_t> lock(mutex_);
    colors_.at(static_cast<size_t>(color_level)) = to_string_(color);
}
#endif

template<typename ConsoleMutex>
SPDLOG_INLINE void ansicolor_sink<ConsoleMutex>::log(const details::log_msg &msg)
{
    // Wrap the originally formatted message in color codes.
    // If color is not supported in the terminal, log as is instead.
    std::lock_guard<mutex_t> lock(mutex_);
    msg.color_range_start = 0;
    msg.color_range_end   = 0;

#if !defined(_WIN32) && defined(SPDLOG_EXTENDED_STLYING)
    msg.styling_ranges.clear();
#endif

    memory_buf_t formatted;
    formatter_->format(msg, formatted);

#if !defined(_WIN32) && defined(SPDLOG_EXTENDED_STLYING)
    if (should_do_colors_ && msg.styling_ranges.size() > 0)
    {
        string_view_t reset = details::styling_table[(int)details::style_type::reset];

        size_t open_styles_count  = 0;
        size_t before_style_range = 0;

        for(auto info = msg.styling_ranges.begin(); info != msg.styling_ranges.end(); info++)
        {
            if (info->is_start)
            {
                // this stlying formatter is the first occurrance past the previous closing formatter
                // all of the preceding characters if any are outside color the range
                if (msg.color_range_end <= info->position && open_styles_count == 0)
                {
                    msg.color_range_start = info->position;
                    // before style range
                    print_range_(formatted, before_style_range, msg.color_range_start);
                }
                open_styles_count++;

                // each styling formatter can contain multiple styles in its style spec
                // loop through each style and print the style code
                auto styles = info->styles;

                // it is possible to not provide a style spec with the formatter, in that
                // case the first style spec will be null_style therefore default to the
                // generic log level style
                if (styles[0] == details::style_type::null_style)
                {
                    styles = colors_.at(msg.level);
                }

                try
                {
                    for(int i = 0; styles[i] != details::style_type::null_style ; i++)
                    {
                        //  in style range
                        print_ccode_(details::styling_table.at((int)styles[i]));
                    }
                }
                catch (const std::out_of_range &ex)
                {   // full style spec output looks either amazing or trash
                }
            }
            else {
                // this closing styling formatter is the first occurrance past the previous
                // n opening styling formatters, because a reset ends all preceding styling
                // this position is the true closing formatter in the event more closing
                // formatters are declared
                if (open_styles_count > 0)
                {
                    msg.color_range_end = info->position;
                    // style range has fully been defined
                    print_range_(formatted, msg.color_range_start, msg.color_range_end);
                    print_ccode_(reset);
                    // get new location outside style ranges
                    before_style_range = msg.color_range_end;
                    // reset open styles count
                    open_styles_count  = 0;
                }
            }
        }
        // after all style ranges
        print_range_(formatted, msg.color_range_end, formatted.size());

        // printing a reset code at the very end of the log clears all dangling
        // open formatters and will not be visible on the output line in the
        // event there are no dangling formatters
        print_ccode_(reset);
    }
#else
    if (should_do_colors_ && msg.color_range_end > msg.color_range_start)
    {
        // before color range
        print_range_(formatted, 0, msg.color_range_start);
        // in color range
        print_ccode_(colors_.at(static_cast<size_t>(msg.level)));
        print_range_(formatted, msg.color_range_start, msg.color_range_end);
        print_ccode_(reset);
        // after color range
        print_range_(formatted, msg.color_range_end, formatted.size());
    }
#endif
    else // no color
    {
        print_range_(formatted, 0, formatted.size());
    }
    fflush(target_file_);
}

template<typename ConsoleMutex>
SPDLOG_INLINE void ansicolor_sink<ConsoleMutex>::flush()
{
    std::lock_guard<mutex_t> lock(mutex_);
    fflush(target_file_);
}

template<typename ConsoleMutex>
SPDLOG_INLINE void ansicolor_sink<ConsoleMutex>::set_pattern(const std::string &pattern)
{
    std::lock_guard<mutex_t> lock(mutex_);
    formatter_ = std::unique_ptr<spdlog::formatter>(new pattern_formatter(pattern));
}

template<typename ConsoleMutex>
SPDLOG_INLINE void ansicolor_sink<ConsoleMutex>::set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter)
{
    std::lock_guard<mutex_t> lock(mutex_);
    formatter_ = std::move(sink_formatter);
}

template<typename ConsoleMutex>
SPDLOG_INLINE bool ansicolor_sink<ConsoleMutex>::should_color()
{
    return should_do_colors_;
}

template<typename ConsoleMutex>
SPDLOG_INLINE void ansicolor_sink<ConsoleMutex>::set_color_mode(color_mode mode)
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
    default:
        should_do_colors_ = false;
    }
}

template<typename ConsoleMutex>
SPDLOG_INLINE void ansicolor_sink<ConsoleMutex>::print_ccode_(const string_view_t &color_code)
{
    fwrite(color_code.data(), sizeof(char), color_code.size(), target_file_);
}

template<typename ConsoleMutex>
SPDLOG_INLINE void ansicolor_sink<ConsoleMutex>::print_range_(const memory_buf_t &formatted, size_t start, size_t end)
{
    fwrite(formatted.data() + start, sizeof(char), end - start, target_file_);
}

template<typename ConsoleMutex>
SPDLOG_INLINE std::string ansicolor_sink<ConsoleMutex>::to_string_(const string_view_t &sv)
{
    return std::string(sv.data(), sv.size());
}

// ansicolor_stdout_sink
template<typename ConsoleMutex>
SPDLOG_INLINE ansicolor_stdout_sink<ConsoleMutex>::ansicolor_stdout_sink(color_mode mode)
    : ansicolor_sink<ConsoleMutex>(stdout, mode)
{}

// ansicolor_stderr_sink
template<typename ConsoleMutex>
SPDLOG_INLINE ansicolor_stderr_sink<ConsoleMutex>::ansicolor_stderr_sink(color_mode mode)
    : ansicolor_sink<ConsoleMutex>(stderr, mode)
{}

} // namespace sinks
} // namespace spdlog
