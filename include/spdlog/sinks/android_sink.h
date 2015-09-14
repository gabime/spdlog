/*************************************************************************/
/* spdlog - an extremely fast and easy to use c++11 logging library.     */
/* Copyright (c) 2014 Gabi Melman.                                       */
/* Copyright (c) 2015 Ruslan Baratov.                                    */
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

#if defined(__ANDROID__)

#include <mutex>
#include "base_sink.h"
#include "../details/null_mutex.h"

#include <android/log.h>

namespace spdlog
{
namespace sinks
{
/*
* Android sink (logging using __android_log_write)
*/
template<class Mutex>
class base_android_sink : public base_sink < Mutex >
{
public:
    explicit base_android_sink(std::string tag="spdlog"): _tag(tag)
    {
    }

    void flush() override
    {
    }

protected:
    void _sink_it(const details::log_msg& msg) override
    {
        const android_LogPriority priority = convert_to_android(msg.level);
        const int expected_size = msg.formatted.size();
        const int size = __android_log_write(
            priority, _tag.c_str(), msg.formatted.c_str()
        );
        if (size > expected_size)
        {
            // Will write a little bit more than original message
        }
        else
        {
            throw spdlog_ex("Send to Android logcat failed");
        }
    }

private:
    static android_LogPriority convert_to_android(spdlog::level::level_enum level)
    {
        switch(level)
        {
            case spdlog::level::trace: return ANDROID_LOG_VERBOSE;
            case spdlog::level::debug: return ANDROID_LOG_DEBUG;
            case spdlog::level::info: return ANDROID_LOG_INFO;
            case spdlog::level::notice: return ANDROID_LOG_INFO;
            case spdlog::level::warn: return ANDROID_LOG_WARN;
            case spdlog::level::err: return ANDROID_LOG_ERROR;
            case spdlog::level::critical: return ANDROID_LOG_FATAL;
            case spdlog::level::alert: return ANDROID_LOG_FATAL;
            case spdlog::level::emerg: return ANDROID_LOG_FATAL;
            default: throw spdlog_ex("Incorrect level value");
        }
    }

    std::string _tag;
};

typedef base_android_sink<std::mutex> android_sink_mt;
typedef base_android_sink<details::null_mutex> android_sink_st;

}
}

#endif
