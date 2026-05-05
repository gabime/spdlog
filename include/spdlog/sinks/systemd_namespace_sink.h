// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/details/null_mutex.h>
#include <spdlog/details/os.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/sinks/base_sink.h>

#include <array>
#ifndef SD_JOURNAL_SUPPRESS_LOCATION
#define SD_JOURNAL_SUPPRESS_LOCATION
#endif
#include <systemd/sd-journal.h>
#include <systemd/sd-daemon.h>

namespace spdlog {
namespace sinks {

/**
 * Sink that opens a stream to a specific systemd journal namespace using
 * sd_journal_stream_fd_with_namespace(). This sink cannot utilize extra fields like TID, CODE_LINE,
 * or CODE_FUNC since sd_journal_send() does not have a namespace-specific counterpart. Journal does
 * interpret kernel-style log level prefixes though, so PRIORITY field is populated correctly.
 */
template <typename Mutex>
class systemd_namespace_sink : public base_sink<Mutex> {
public:
    explicit systemd_namespace_sink(std::string ident,
                                    std::string name_space,
                                    bool enable_formatting)
        : enable_formatting_{enable_formatting},
          level_prefixes_{{/* spdlog::level::trace      */ SD_DEBUG,
                           /* spdlog::level::debug      */ SD_DEBUG,
                           /* spdlog::level::info       */ SD_INFO,
                           /* spdlog::level::warn       */ SD_WARNING,
                           /* spdlog::level::err        */ SD_ERR,
                           /* spdlog::level::critical   */ SD_CRIT,
                           /* spdlog::level::off        */ SD_INFO}} {
        int stream_fd = -1;
        if (name_space.empty()) {
            stream_fd = ::sd_journal_stream_fd(ident.c_str(), LOG_INFO, 1);
        } else {
            stream_fd = ::sd_journal_stream_fd_with_namespace(name_space.c_str(), ident.c_str(),
                                                              LOG_INFO, 1);
        }
        if (stream_fd < 0) {
            throw_spdlog_ex("Failed opening systemd journal stream to namespace '" + name_space +
                            "': " + ::strerror(-stream_fd));
        }
        journal_ = ::fdopen(stream_fd, "w");
        if (journal_ == nullptr) {
            // Capture errno from the failed fdopen() instead of close() (in case that fails too)
            const int saved_errno = errno;
            ::close(stream_fd);
            throw_spdlog_ex(
                "Failed opening systemd journal stream to namespace '" + name_space + "'",
                saved_errno);
        }
        // Use line buffering which matches with journald's line-oriented protocol
        ::setvbuf(journal_, nullptr, _IOLBF, 0);
    }

    ~systemd_namespace_sink() override {
        if (journal_ != nullptr) {
            ::fclose(journal_);
            journal_ = nullptr;
        }
    }

    systemd_namespace_sink(const systemd_namespace_sink &) = delete;
    systemd_namespace_sink &operator=(const systemd_namespace_sink &) = delete;

protected:
    FILE *journal_ = nullptr;
    bool enable_formatting_ = false;
    using level_prefix_array = std::array<const char *, 7>;
    level_prefix_array level_prefixes_;

    void sink_it_(const details::log_msg &msg) override {
        string_view_t payload;
        memory_buf_t formatted;
        if (enable_formatting_) {
            base_sink<Mutex>::formatter_->format(msg, formatted);
            payload = string_view_t(formatted.data(), formatted.size());
        } else {
            payload = msg.payload;
        }

        // Journal stream is line-oriented; if there's newlines in the payload, send each
        // newline-delimited piece separately as its own message.
        size_t pos = 0;
        while (pos < payload.size()) {
            size_t nl_pos = payload.find('\n', pos);
            size_t end = (nl_pos == std::string_view::npos) ? payload.size() : nl_pos + 1;
            std::string_view one_message = payload.substr(pos, end - pos);

            // Limit single message size to max int, but take into account the 3 characters of
            // kernel-style log level prefix
            size_t length = one_message.size();
            if ((length + 3) > static_cast<size_t>(std::numeric_limits<int>::max())) {
                length = static_cast<size_t>(std::numeric_limits<int>::max()) - 3;
            }

            // Write log level prefix
            details::os::fwrite_bytes(
                level_prefixes_.at(static_cast<level_prefix_array::size_type>(msg.level)), 3,
                journal_);
            // Write the message
            details::os::fwrite_bytes(one_message.data(), one_message.size(), journal_);
            // Append newline if the message didn't have one
            if (one_message.empty() || one_message.back() != '\n') {
                details::os::fwrite_bytes("\n", 1, journal_);
            }

            pos = end;
        }
    }

    void flush_() override {}
};

using systemd_namespace_sink_mt = systemd_namespace_sink<std::mutex>;
using systemd_namespace_sink_st = systemd_namespace_sink<details::null_mutex>;
}  // namespace sinks

// Create and register a syslog logger
template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> systemd_namespace_logger_mt(const std::string &logger_name,
                                                           const std::string &ident,
                                                           const std::string &name_space,
                                                           bool enable_formatting) {
    return Factory::template create<sinks::systemd_namespace_sink_mt>(
        logger_name, ident, name_space, enable_formatting);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> systemd_namespace_logger_st(const std::string &logger_name,
                                                           const std::string &ident,
                                                           const std::string &name_space,
                                                           bool enable_formatting) {
    return Factory::template create<sinks::systemd_namespace_sink_st>(
        logger_name, ident, name_space, enable_formatting);
}
}  // namespace spdlog
