//
// Copyright(c) 2016 Kevin M. Godby (modified version by spdlog).
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <spdlog/sinks/base_sink.h>
#include <spdlog/common.h>

#include <string>
#include <map>

namespace spdlog {
namespace sinks {

/**
 * @brief The ansi_color_sink is a decorator around another sink and prefixes
 * the output with an ANSI escape sequence color code depending on the severity
 * of the message.
 */
class ansicolor_sink : public sink {
public:
    ansicolor_sink(sink_ptr wrapped_sink);
    virtual ~ansicolor_sink();

    ansicolor_sink(const ansicolor_sink& other);
    ansicolor_sink& operator=(const ansicolor_sink& other);

    virtual void log(const details::log_msg& msg) override;
    virtual void flush() override;

    void set_color(level::level_enum level, const std::string& color);

    /// \name Formatting codes
    //@{
    const std::string reset      = "\033[00m";
    const std::string bold       = "\033[1m";
    const std::string dark       = "\033[2m";
    const std::string underline  = "\033[4m";
    const std::string blink      = "\033[5m";
    const std::string reverse    = "\033[7m";
    const std::string concealed  = "\033[8m";
    //@}

    /// \name Foreground colors
    //@{
    const std::string grey       = "\033[30m";
    const std::string red        = "\033[31m";
    const std::string green      = "\033[32m";
    const std::string yellow     = "\033[33m";
    const std::string blue       = "\033[34m";
    const std::string magenta    = "\033[35m";
    const std::string cyan       = "\033[36m";
    const std::string white      = "\033[37m";
    //@}

    /// \name Background colors
    //@{
    const std::string on_grey    = "\033[40m";
    const std::string on_red     = "\033[41m";
    const std::string on_green   = "\033[42m";
    const std::string on_yellow  = "\033[43m";
    const std::string on_blue    = "\033[44m";
    const std::string on_magenta = "\033[45m";
    const std::string on_cyan    = "\033[46m";
    const std::string on_white   = "\033[47m";
    //@}


protected:
    sink_ptr sink_;
    std::map<level::level_enum, std::string> colors_;
};

inline ansicolor_sink::ansicolor_sink(sink_ptr wrapped_sink) : sink_(wrapped_sink)
{
    colors_[level::trace]    = white;
    colors_[level::debug]    = white;
    colors_[level::info]     = white;
    colors_[level::notice]   = bold + white;
    colors_[level::warn]     = bold + yellow;
    colors_[level::err]      = red;
    colors_[level::critical] = bold + red;
    colors_[level::alert]    = bold + white + on_red;
    colors_[level::emerg]    = bold + yellow + on_red;
    colors_[level::off]      = reset;
}

inline ansicolor_sink::~ansicolor_sink()
{
    flush();
}

inline ansicolor_sink::ansicolor_sink(const ansicolor_sink& other) : sink_(other.sink_), colors_(other.colors_)
{
    // do nothing
}


inline ansicolor_sink& ansicolor_sink::operator=(const ansicolor_sink& other)
{
    if (this == &other)
        return *this;

    sink_ = other.sink_;
    colors_ = other.colors_;
    return *this;
}

inline void ansicolor_sink::log(const details::log_msg& msg)
{
    // Wrap the originally formatted message in color codes
    const std::string prefix = colors_[msg.level];
    const std::string s = msg.formatted.str();
    const std::string suffix = reset;
    details::log_msg m;
    m.formatted << prefix  << s << suffix;
    sink_->log(m);
}

inline void ansicolor_sink::flush()
{
    sink_->flush();
}

inline void ansicolor_sink::set_color(level::level_enum level, const std::string& color)
{
    colors_[level] = color;
}

} // namespace sinks
} // namespace spdlog

