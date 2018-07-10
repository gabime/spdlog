//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#if defined(__ANDROID__)

#include "spdlog/details/os.h"
#include "spdlog/sinks/sink.h"

#include <android/log.h>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>

#if !defined(SPDLOG_ANDROID_RETRIES)
#define SPDLOG_ANDROID_RETRIES 2
#endif

namespace spdlog {
namespace sinks {

/*
 * Android sink (logging using __android_log_write)
 * __android_log_write is thread-safe. No lock is needed.
 */
class android_sink : public sink
{
public:
    explicit android_sink(const std::string &tag = "spdlog", bool use_raw_msg = false)
        : tag_(tag)
        , use_raw_msg_(use_raw_msg)
    {
    }

    void log(const details::log_msg &msg) override
    {
        const android_LogPriority priority = convert_to_android_(msg.level);
        fmt::memory_buffer formatted;
        if (use_raw_msg_)
        {
            formatted.append(msg.raw.data(), msg.raw.data() + msg.raw.size());
        }
        else
        {
            formatter_->format(msg, formatted);
        }
        formatted.push_back('\0');
        const char *msg_output = formatted.data();

        // See system/core/liblog/logger_write.c for explanation of return value
        int ret = __android_log_write(priority, tag_.c_str(), msg_output);
        int retry_count = 0;
        while ((ret == -11 /*EAGAIN*/) && (retry_count < SPDLOG_ANDROID_RETRIES))
        {
            details::os::sleep_for_millis(5);
            ret = __android_log_write(priority, tag_.c_str(), msg_output);
            retry_count++;
        }

        if (ret < 0)
        {
            throw spdlog_ex("__android_log_write() failed", ret);
        }
    }

    void flush() override {}

private:
    static android_LogPriority convert_to_android_(spdlog::level::level_enum level)
    {
        switch (level)
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

    std::string tag_;
    bool use_raw_msg_;
};

} // namespace sinks

// Create and register android syslog logger

template<typename Factory = default_factory>
inline std::shared_ptr<logger> android_logger(const std::string &logger_name, const std::string &tag = "spdlog")
{
    return return Factory::template create<sinks::android_sink>(logger_name, tag);
}

} // namespace spdlog

#endif
