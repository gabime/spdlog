//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#if defined(__ANDROID__)

#include "../details/os.h"
#include "sink.h"

#include <android/log.h>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>

#if !defined(SPDLOG_ANDROID_RETRIES)
#define SPDLOG_ANDROID_RETRIES 2
#endif

namespace spdlog {
namespace sinks {

/*
 * Android sink
 * (logging using __android_log_write or __android_log_buf_write depending on the specified BufferID)
 * They are thread-safe. No lock is needed.
 */
template<int BufferID>
class android_sink_buf : public sink
{
public:
    explicit android_sink_buf(const std::string &tag = "spdlog", bool use_raw_msg = false)
        : _tag(tag)
        , _use_raw_msg(use_raw_msg)
    {
    }

    void log(const details::log_msg &msg) override
    {
        const android_LogPriority priority = convert_to_android(msg.level);
        const char *msg_output = (_use_raw_msg ? msg.raw.c_str() : msg.formatted.c_str());

        // See system/core/liblog/logger_write.c for explanation of return value
        int ret = android_log(priority, _tag.c_str(), msg_output);
        int retry_count = 0;
        while ((ret == -11 /*EAGAIN*/) && (retry_count < SPDLOG_ANDROID_RETRIES))
        {
            details::os::sleep_for_millis(5);
            ret = android_log(priority, _tag.c_str(), msg_output);
            retry_count++;
        }

        if (ret < 0)
        {
            throw spdlog_ex("__android_log_write() failed", ret);
        }
    }

    void flush() override {}

private:
    // There might be liblog versions used, that do not support __android_log_buf_write. So we only compile and link against
    // __android_log_buf_write, if user explicitely provides a non-default log buffer. Otherwise, when using the default log buffer, always
    // log via __android_log_write.
    template<int ID = BufferID, typename... Args>
    typename std::enable_if<ID == static_cast<int>(log_id::LOG_ID_MAIN), int>::type android_log(Args... args)
    {
        return __android_log_write(std::forward<Args>(args)...);
    }

    template<int ID = BufferID, typename... Args>
    typename std::enable_if<ID != static_cast<int>(log_id::LOG_ID_MAIN), int>::type android_log(Args... args)
    {
        return __android_log_buf_write(ID, std::forward<Args>(args)...);
    }

    static android_LogPriority convert_to_android(spdlog::level::level_enum level)
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

    std::string _tag;
    bool _use_raw_msg;
};

/**
 * Android sink (logging using __android_log_write)
 * __android_log_write is thread-safe. No lock is needed.
 */
using android_sink = android_sink_buf<log_id::LOG_ID_MAIN>;

} // namespace sinks
} // namespace spdlog

#endif
