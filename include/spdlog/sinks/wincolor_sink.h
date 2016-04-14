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

/**
 * @brief The wincolor_sink is a decorator around another sink and uses
 * the windows api to set the color depending on the severity
 * of the message.
 */
template<class Mutex>
class wincolor_sink : public base_sink<Mutex>
{
public:
    wincolor_sink(sink_ptr wrapped_sink);
    virtual ~wincolor_sink();

    wincolor_sink(const wincolor_sink& other) = delete;
    wincolor_sink& operator=(const wincolor_sink& other) = delete;

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

    void set_color( level::level_enum level, const short& color );
    sink_ptr& wrapped_sink();

protected:
    virtual void _sink_it(const details::log_msg& msg) override;

    sink_ptr sink_;
    std::map<level::level_enum, short> colors_;
};

typedef wincolor_sink<details::null_mutex> wincolor_sink_st;
typedef wincolor_sink<std::mutex> wincolor_sink_mt;

template<class Mutex>
inline wincolor_sink<Mutex>::wincolor_sink(sink_ptr wrapped_sink) : sink_(wrapped_sink)
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

template<class Mutex>
inline void wincolor_sink<Mutex>::_sink_it( const details::log_msg& msg )
{
    // Wrap the originally formatted message in color codes
    SetConsoleTextAttribute(GetStdHandle( STD_OUTPUT_HANDLE ), colors_[msg.level]);
    SetConsoleTextAttribute(GetStdHandle( STD_ERROR_HANDLE ), colors_[msg.level]);
    sink_->log( msg );
    SetConsoleTextAttribute(GetStdHandle( STD_ERROR_HANDLE ), reset);
    SetConsoleTextAttribute(GetStdHandle( STD_OUTPUT_HANDLE ), reset);
}

template<class Mutex>
inline void wincolor_sink<Mutex>::flush()
{
    sink_->flush();
}

template<class Mutex>
inline void wincolor_sink<Mutex>::set_color( level::level_enum level, const short& color )
{
    colors_[level] = color;
}

template<class Mutex>
inline sink_ptr& wincolor_sink<Mutex>::wrapped_sink()
{
    return sink_;
}

template<class Mutex>
inline wincolor_sink<Mutex>::~wincolor_sink()
{
    flush();
}

} // namespace sinks
} // namespace spdlog

