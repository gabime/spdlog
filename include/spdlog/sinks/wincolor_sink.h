//
// Copyright(c) 2016 Christopher J. Torres (a modified verison of ansicolor_sink).
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

#ifndef _WIN32
#define FOREGROUND_INTENSITY 0
#define FOREGROUND_RED 0
#define FOREGROUND_GREEN 0
#define FOREGROUND_BLUE 0
#define BACKGROUND_INTENSITY 0
#define BACKGROUND_RED 0
#define BACKGROUND_GREEN 0
#define BACKGROUND_BLUE 0
#endif

/**
 * @brief The wincolor_sink is a decorator around another sink and uses
 * the windows api to set the color depending on the severity
 * of the message.
 */
class wincolor_sink : public sink
{
public:
    wincolor_sink(sink_ptr wrapped_sink);
    virtual ~wincolor_sink();

    wincolor_sink(const wincolor_sink& other) = delete;
    wincolor_sink& operator=(const wincolor_sink& other) = delete;

    virtual void log(const details::log_msg& msg) override;
    virtual void flush() override;

    // Formatting codes
    const short reset      = 0;
    const short bold       = FOREGROUND_INTENSITY;
    const short dark       = reset; // Not implemented in windows
    const short underline  = reset; // Not implemented in windows
    const short blink      = reset; // Not implemented in windows
    const short reverse    = FOREGROUND_RED | FOREGROUND_GREEN |
                                FOREGROUND_BLUE | BACKGROUND_RED |
                                BACKGROUND_GREEN | BACKGROUND_BLUE; // XOR to use this
    const short concealed  = reset; // Not implemented in windows

    // Foreground colors
    const short grey       = bold;
    const short red        = FOREGROUND_RED;
    const short green      = FOREGROUND_GREEN;
    const short yellow     = FOREGROUND_RED | FOREGROUND_GREEN;
    const short blue       = FOREGROUND_BLUE;
    const short magenta    = FOREGROUND_RED | FOREGROUND_BLUE;
    const short cyan       = FOREGROUND_GREEN | FOREGROUND_BLUE;
    const short white      = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

    /// Background colors
    const short on_grey    = BACKGROUND_INTENSITY;
    const short on_red     = BACKGROUND_RED;
    const short on_green   = BACKGROUND_GREEN;
    const short on_yellow  = BACKGROUND_RED | BACKGROUND_GREEN;
    const short on_blue    = BACKGROUND_BLUE;
    const short on_magenta = BACKGROUND_RED | BACKGROUND_BLUE;
    const short on_cyan    = BACKGROUND_GREEN | BACKGROUND_BLUE;
    const short on_white   = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;

    void set_color(level::level_enum level, const short& color);
    sink_ptr& wrapped_sink();

protected:
    sink_ptr sink_;
    std::map<level::level_enum, short> colors_;
};

inline wincolor_sink::wincolor_sink(sink_ptr wrapped_sink) : sink_(wrapped_sink)
{
    colors_[level::trace]    = cyan;
    colors_[level::debug]    = cyan;
    colors_[level::info]     = white;
    colors_[level::err]      = red;
    colors_[level::off]      = reset;

    colors_[level::notice]   = bold | white;
    colors_[level::warn]     = bold | yellow;
    colors_[level::critical] = bold | red;
    colors_[level::alert]    = bold | white | on_red;
    colors_[level::emerg]    = bold | yellow | on_red;
}

inline void wincolor_sink::log(const details::log_msg& msg)
{
    // Wrap the originally formatted message in color codes
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle( STD_OUTPUT_HANDLE ), colors_[msg.level]);
    SetConsoleTextAttribute(GetStdHandle( STD_ERROR_HANDLE ), colors_[msg.level]);
#endif

    sink_->log( msg );

#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle( STD_ERROR_HANDLE ), reset);
    SetConsoleTextAttribute(GetStdHandle( STD_OUTPUT_HANDLE ), reset);
#endif
}

inline void wincolor_sink::flush()
{
    sink_->flush();
}

inline void wincolor_sink::set_color(level::level_enum level, const short& color)
{
    colors_[level] = color;
}

inline sink_ptr& wincolor_sink::wrapped_sink()
{
    return sink_;
}

inline wincolor_sink::~wincolor_sink()
{
    flush();
}

} // namespace sinks
} // namespace spdlog

