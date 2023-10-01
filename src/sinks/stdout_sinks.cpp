// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/sinks/stdout_sinks.h"

#include <memory>

#include "spdlog/pattern_formatter.h"

// clang-format off
#ifdef _WIN32
    // under windows using fwrite to non-binary stream results in \r\r\n (see issue #1675)
    // so instead we use ::FileWrite
    #include "spdlog/details/windows_include.h"

    #ifndef _USING_V110_SDK71_  // fileapi.h doesn't exist in winxp
        #include <fileapi.h>    // WriteFile (..)
    #endif

    #include <io.h>     // _get_osfhandle(..)
    #include <stdio.h>  // _fileno(..)
#endif                  // _WIN32

// clang-format on
namespace spdlog {

namespace sinks {

template <typename Mutex>
stdout_sink_base<Mutex>::stdout_sink_base(FILE *file)
    : file_(file) {
#ifdef _WIN32
    // get windows handle from the FILE* object

    handle_ = reinterpret_cast<HANDLE>(::_get_osfhandle(::_fileno(file_)));

    // don't throw to support cases where no console is attached,
    // and let the log method to do nothing if (handle_ == INVALID_HANDLE_VALUE).
    // throw only if non stdout/stderr target is requested (probably regular file and not console).
    if (handle_ == INVALID_HANDLE_VALUE && file != stdout && file != stderr) {
        throw_spdlog_ex("spdlog::stdout_sink_base: _get_osfhandle() failed", errno);
    }
#endif  // WIN32
}

template <typename Mutex>
void stdout_sink_base<Mutex>::sink_it_(const details::log_msg &msg) {
#ifdef _WIN32
    if (handle_ == INVALID_HANDLE_VALUE) {
        return;
    }

    memory_buf_t formatted;
    base_sink<Mutex>::formatter_->format(msg, formatted);
    auto size = static_cast<DWORD>(formatted.size());
    DWORD bytes_written = 0;
    bool ok = ::WriteFile(handle_, formatted.data(), size, &bytes_written, nullptr) != 0;
    if (!ok) {
        throw_spdlog_ex("stdout_sink_base: WriteFile() failed. GetLastError(): " +
                        std::to_string(::GetLastError()));
    }
#else
    memory_buf_t formatted;
    base_sink<Mutex>::formatter_->format(msg, formatted);
    ::fwrite(formatted.data(), sizeof(char), formatted.size(), file_);
#endif                // WIN32
    ::fflush(file_);  // flush every line to terminal
}

template <typename Mutex>
void stdout_sink_base<Mutex>::flush_() {
    fflush(file_);
}

// stdout sink
template <typename Mutex>
stdout_sink<Mutex>::stdout_sink()
    : stdout_sink_base<Mutex>(stdout) {}

// stderr sink
template <typename Mutex>
stderr_sink<Mutex>::stderr_sink()
    : stdout_sink_base<Mutex>(stderr) {}

}  // namespace sinks

// factory methods
template <typename Factory>
std::shared_ptr<logger> stdout_logger_mt(const std::string &logger_name) {
    return Factory::template create<sinks::stdout_sink_mt>(logger_name);
}

template <typename Factory>
std::shared_ptr<logger> stdout_logger_st(const std::string &logger_name) {
    return Factory::template create<sinks::stdout_sink_st>(logger_name);
}

template <typename Factory>
std::shared_ptr<logger> stderr_logger_mt(const std::string &logger_name) {
    return Factory::template create<sinks::stderr_sink_mt>(logger_name);
}

template <typename Factory>
std::shared_ptr<logger> stderr_logger_st(const std::string &logger_name) {
    return Factory::template create<sinks::stderr_sink_st>(logger_name);
}
}  // namespace spdlog

// template instantiations for stdout/stderr loggers
template class SPDLOG_API spdlog::sinks::stdout_sink_base<std::mutex>;
template class SPDLOG_API spdlog::sinks::stdout_sink_base<spdlog::details::null_mutex>;
template class SPDLOG_API spdlog::sinks::stdout_sink<std::mutex>;
template class SPDLOG_API spdlog::sinks::stdout_sink<spdlog::details::null_mutex>;
template class SPDLOG_API spdlog::sinks::stderr_sink<std::mutex>;
template class SPDLOG_API spdlog::sinks::stderr_sink<spdlog::details::null_mutex>;

// template instantiations for stdout/stderr factory functions
#include "spdlog/async.h"
#include "spdlog/details/synchronous_factory.h"

template SPDLOG_API std::shared_ptr<spdlog::logger>
spdlog::stdout_logger_mt<spdlog::synchronous_factory>(const std::string &logger_name);
template SPDLOG_API std::shared_ptr<spdlog::logger>
spdlog::stdout_logger_st<spdlog::synchronous_factory>(const std::string &logger_name);
template SPDLOG_API std::shared_ptr<spdlog::logger>
spdlog::stderr_logger_mt<spdlog::synchronous_factory>(const std::string &logger_name);
template SPDLOG_API std::shared_ptr<spdlog::logger>
spdlog::stderr_logger_st<spdlog::synchronous_factory>(const std::string &logger_name);

template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stdout_logger_mt<spdlog::async_factory>(
    const std::string &logger_name);
template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stdout_logger_st<spdlog::async_factory>(
    const std::string &logger_name);
template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stderr_logger_mt<spdlog::async_factory>(
    const std::string &logger_name);
template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stderr_logger_st<spdlog::async_factory>(
    const std::string &logger_name);
