/*************************************************************************/
/* spdlog - an extremely fast and easy to use c++11 logging library.     */
/* Copyright (c) 2014 Gabi Melman.                                       */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once
#include <type_traits>
#include "../common.h"
#include "../logger.h"

// Line logger class - aggregates operator<< calls to fast ostream
// and logs upon destruction

namespace spdlog
{
namespace details
{
class line_logger
{
public:
    line_logger(logger* callback_logger, level::level_enum msg_level, bool enabled):
        _callback_logger(callback_logger),
        _log_msg(msg_level),
        _enabled(enabled)
    {}

    // No copy intended. Only move
    line_logger(const line_logger& other) = delete;
    line_logger& operator=(const line_logger&) = delete;
    line_logger& operator=(line_logger&&) = delete;


    line_logger(line_logger&& other) :
        _callback_logger(other._callback_logger),
        _log_msg(std::move(other._log_msg)),
        _enabled(other._enabled)
    {
        other.disable();
    }

    //Log the log message using the callback logger
    ~line_logger()
    {
        if (_enabled)
        {
#ifndef SPDLOG_NO_NAME
            _log_msg.logger_name = _callback_logger->name();
#endif
#ifndef SPDLOG_NO_DATETIME
            _log_msg.time = os::now();
#endif

#ifndef SPDLOG_NO_THREAD_ID
            _log_msg.thread_id = os::thread_id();
#endif
            _callback_logger->_log_msg(_log_msg);
        }
    }

    //
    // Support for format string with variadic args
    //


    void write(const char* what)
    {
        if (_enabled)
            _log_msg.raw << what;
    }

    template <typename... Args>
    void write(const char* fmt, const Args&... args)
    {
        if (!_enabled)
            return;
        try
        {
            _log_msg.raw.write(fmt, args...);
        }
        catch (const fmt::FormatError& e)
        {
            throw spdlog_ex(fmt::format("formatting error while processing format string '{}': {}", fmt, e.what()));
        }
    }


    //
    // Support for operator<<
    //
    line_logger& operator<<(const char* what)
    {
        if (_enabled)
            _log_msg.raw << what;
        return *this;
    }

    line_logger& operator<<(const std::string& what)
    {
        if (_enabled)
            _log_msg.raw << what;
        return *this;
    }

    line_logger& operator<<(int what)
    {
        if (_enabled)
            _log_msg.raw << what;
        return *this;
    }

    line_logger& operator<<(unsigned int what)
    {
        if (_enabled)
            _log_msg.raw << what;
        return *this;
    }


    line_logger& operator<<(long what)
    {
        if (_enabled)
            _log_msg.raw << what;
        return *this;
    }

    line_logger& operator<<(unsigned long what)
    {
        if (_enabled)
            _log_msg.raw << what;
        return *this;
    }

    line_logger& operator<<(long long what)
    {
        if (_enabled)
            _log_msg.raw << what;
        return *this;
    }

    line_logger& operator<<(unsigned long long what)
    {
        if (_enabled)
            _log_msg.raw << what;
        return *this;
    }

    line_logger& operator<<(double what)
    {
        if (_enabled)
            _log_msg.raw << what;
        return *this;
    }

    line_logger& operator<<(long double what)
    {
        if (_enabled)
            _log_msg.raw << what;
        return *this;
    }

    line_logger& operator<<(float what)
    {
        if (_enabled)
            _log_msg.raw << what;
        return *this;
    }

    line_logger& operator<<(char what)
    {
        if (_enabled)
            _log_msg.raw << what;
        return *this;
    }

    //Support user types which implements operator<<
    template<typename T>
    line_logger& operator<<(const T& what)
    {
        if (_enabled)
            _log_msg.raw.write("{}", what);
        return *this;
    }


    void disable()
    {
        _enabled = false;
    }

    bool is_enabled() const
    {
        return _enabled;
    }


private:
    logger* _callback_logger;
    log_msg _log_msg;
    bool _enabled;
};
} //Namespace details
} // Namespace spdlog
