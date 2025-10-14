#pragma once

#ifdef __EMSCRIPTEN__

#include <spdlog/details/fmt_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/os.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/sinks/base_sink.h>

#include <emscripten/emscripten.h>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>


namespace spdlog {
namespace sinks {

/*
 * Emscripten sink
 * (logging using emscripten_log)
 */

template<typename Mutex>
class emscripten_sink : public base_sink <Mutex>
{
    void sink_it_(const details::log_msg& msg) override
    {
        // log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
        // msg.payload (before v1.3.0: msg.raw) contains pre formatted log

        // If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        emscripten_log(EM_LOG_CONSOLE | convert_to_emscripten_(msg.level), "%s", fmt::to_string(formatted).c_str());
    }

    void flush_() override {}

private:
    static int convert_to_emscripten_(spdlog::level::level_enum level) {
        switch (level) {
            case spdlog::level::trace:
            case spdlog::level::debug:
                return EM_LOG_DEBUG;
            case spdlog::level::info:
                return EM_LOG_INFO;
            case spdlog::level::warn:
                return EM_LOG_WARN;
            case spdlog::level::err:
            case spdlog::level::critical:
                return EM_LOG_ERROR;
            default:
                return 0;
        }
    }
};

using emscripten_sink_mt = emscripten_sink<std::mutex>;
using emscripten_sink_st = emscripten_sink<details::null_mutex>;

}  // namespace sinks

}  // namespace spdlog

#endif  // __EMSCRIPTEN__
