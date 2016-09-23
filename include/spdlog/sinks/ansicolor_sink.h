//
// Copyright(c) 2016 Kevin M. Godby (a modified version by spdlog).
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <spdlog/sinks/base_sink.h>
#include <spdlog/common.h>

#include <string>
#include <map>

namespace spdlog
{
namespace sinks
{

/**
 * @brief The ansi_color_sink is a decorator around another sink and prefixes
 * the output with an ANSI escape sequence color code depending on the severity
 * of the message.
 */
class ansicolor_sink : public sink
{
public:
    ansicolor_sink(sink_ptr wrapped_sink);
    virtual ~ansicolor_sink();

    ansicolor_sink(const ansicolor_sink& other) = delete;
    ansicolor_sink& operator=(const ansicolor_sink& other) = delete;

    virtual void log(const details::log_msg& msg) override;
    virtual void flush() override;

    void set_color(level::level_enum level, const log_string_t& color);

    /// Formatting codes
    const log_string_t reset      = _SLT("\033[00m");
    const log_string_t bold       = _SLT("\033[1m");
    const log_string_t dark       = _SLT("\033[2m");
    const log_string_t underline  = _SLT("\033[4m");
    const log_string_t blink      = _SLT("\033[5m");
    const log_string_t reverse    = _SLT("\033[7m");
    const log_string_t concealed  = _SLT("\033[8m");

    // Foreground colors
    const log_string_t grey       = _SLT("\033[30m");
    const log_string_t red        = _SLT("\033[31m");
    const log_string_t green      = _SLT("\033[32m");
    const log_string_t yellow     = _SLT("\033[33m");
    const log_string_t blue       = _SLT("\033[34m");
    const log_string_t magenta    = _SLT("\033[35m");
    const log_string_t cyan       = _SLT("\033[36m");
    const log_string_t white      = _SLT("\033[37m");

    /// Background colors
    const log_string_t on_grey    = _SLT("\033[40m");
    const log_string_t on_red     = _SLT("\033[41m");
    const log_string_t on_green   = _SLT("\033[42m");
    const log_string_t on_yellow  = _SLT("\033[43m");
    const log_string_t on_blue    = _SLT("\033[44m");
    const log_string_t on_magenta = _SLT("\033[45m");
    const log_string_t on_cyan    = _SLT("\033[46m");
    const log_string_t on_white   = _SLT("\033[47m");


protected:
    sink_ptr sink_;
    std::map<level::level_enum, log_string_t> colors_;
};

inline ansicolor_sink::ansicolor_sink(sink_ptr wrapped_sink) : sink_(wrapped_sink)
{
    colors_[level::trace]   = cyan;
    colors_[level::debug]   = cyan;
    colors_[level::info]    = bold;
    colors_[level::warn]    = yellow + bold;
    colors_[level::err]     = red + bold;
    colors_[level::critical] = bold + on_red;
    colors_[level::off]      = reset;
}

inline void ansicolor_sink::log(const details::log_msg& msg)
{
    // Wrap the originally formatted message in color codes
    const log_string_t& prefix = colors_[msg.level];
    const log_string_t& s = msg.formatted.str();
    const log_string_t& suffix = reset;
    details::log_msg m;
    m.level = msg.level;
    m.logger_name = msg.logger_name;
    m.time = msg.time;
    m.thread_id = msg.thread_id;
    m.formatted << prefix  << s << suffix;
    sink_->log(m);
}

inline void ansicolor_sink::flush()
{
    sink_->flush();
}

inline void ansicolor_sink::set_color(level::level_enum level, const log_string_t& color)
{
    colors_[level] = color;
}

inline ansicolor_sink::~ansicolor_sink()
{
    flush();
}

} // namespace sinks
} // namespace spdlog

