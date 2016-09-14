//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#if defined(__ANDROID__)

#include <spdlog/sinks/sink.h>

#include <mutex>
#include <string>
#include <android/log.h>

namespace spdlog
{
namespace sinks
{

/*
* Android sink (logging using __android_log_write)
* __android_log_write is thread-safe. No lock is needed.
*/
class android_sink : public sink
{
public:
    explicit android_sink(const std::string& tag = "spdlog"): _tag(tag) {}

    void log(const details::log_msg& msg) override
    {
        const android_LogPriority priority = convert_to_android(msg.level);
        // See system/core/liblog/logger_write.c for explanation of return value
        const int ret = __android_log_write(
                            priority, _tag.c_str(), msg.formatted.c_str()
                        );
        if (ret < 0)
        {
            throw spdlog_ex("__android_log_write() failed", ret);
        }
    }

    void flush() override
    {
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
        case spdlog::level::warn:
            return ANDROID_LOG_WARN;
        case spdlog::level::err:
            return ANDROID_LOG_ERROR;
        case spdlog::level::critical:
            return ANDROID_LOG_FATAL;
        default:
            return ANDROID_LOG_DEFAULT;
        }
    }

    std::string _tag;
};

}
}

#endif
