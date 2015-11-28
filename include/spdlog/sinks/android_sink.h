//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

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
        case spdlog::level::trace:
            return ANDROID_LOG_VERBOSE;
        case spdlog::level::debug:
            return ANDROID_LOG_DEBUG;
        case spdlog::level::info:
            return ANDROID_LOG_INFO;
        case spdlog::level::notice:
            return ANDROID_LOG_INFO;
        case spdlog::level::warn:
            return ANDROID_LOG_WARN;
        case spdlog::level::err:
            return ANDROID_LOG_ERROR;
        case spdlog::level::critical:
            return ANDROID_LOG_FATAL;
        case spdlog::level::alert:
            return ANDROID_LOG_FATAL;
        case spdlog::level::emerg:
            return ANDROID_LOG_FATAL;
        default:
            throw spdlog_ex("Incorrect level value");
        }
    }

    std::string _tag;
};

typedef base_android_sink<std::mutex> android_sink_mt;
typedef base_android_sink<details::null_mutex> android_sink_st;

}
}

#endif
