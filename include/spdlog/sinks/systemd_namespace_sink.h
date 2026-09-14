// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/details/null_mutex.h>
#include <spdlog/details/os.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/sinks/base_sink.h>

#include <array>
#include <unistd.h>
#include <systemd/sd-journal.h>
#include <systemd/sd-daemon.h>

SPDLOG_NAMESPACE_BEGIN
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
        : enable_formatting_{enable_formatting} {
        int stream_fd = -1;
        if (name_space.empty()) {
            stream_fd = ::sd_journal_stream_fd(ident.c_str(), LOG_INFO, 1);
        } else {
            stream_fd = ::sd_journal_stream_fd_with_namespace(name_space.c_str(), ident.c_str(),
                                                              LOG_INFO, 1);
        }
        if (stream_fd < 0) {
            throw_spdlog_ex(
                "Failed opening systemd journal stream to namespace '" + name_space + "'",
                -stream_fd);
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
        if (::setvbuf(journal_, nullptr, _IOLBF, 0) != 0) {
            // Capture errno from the failed setvbuf() instead of close() (in case that fails too)
            const int saved_errno = errno;
            ::close(stream_fd);
            throw_spdlog_ex(
                "Failed setting line buffering mode for systemd journal stream to namespace '" +
                    name_space + "'",
                saved_errno);
        }
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
    using level_prefix_array = std::array<string_view_t, 7>;
    static constexpr level_prefix_array level_prefixes_ = {
        {/* spdlog::level::trace      */ SD_DEBUG,
         /* spdlog::level::debug      */ SD_DEBUG,
         /* spdlog::level::info       */ SD_INFO,
         /* spdlog::level::warn       */ SD_WARNING,
         /* spdlog::level::err        */ SD_ERR,
         /* spdlog::level::critical   */ SD_CRIT,
         /* spdlog::level::off        */ SD_INFO}};

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
            size_t end = (nl_pos == string_view_t::npos) ? payload.size() : nl_pos + 1;
            string_view_t one_message = payload.substr(pos, end - pos);

            // Write log level prefix
            const auto &prefix =
                level_prefixes_.at(static_cast<level_prefix_array::size_type>(msg.level));
            write_or_throw_(prefix.data(), prefix.size());
            // Write the message
            write_or_throw_(one_message.data(), one_message.size());
            // Append newline if the message didn't have one
            if (nl_pos == string_view_t::npos) {
                write_or_throw_("\n", 1);
            }

            pos = end;
        }
    }

    void flush_() override {
        if (journal_ != nullptr) {
            if (::fflush(journal_) != 0) {
                throw_spdlog_ex("Failed flush to systemd journal", errno);
            }
        }
    }

private:
    void write_or_throw_(const void *data, size_t n) {
        if (journal_ != nullptr) {
            if (!details::os::fwrite_bytes(data, n, journal_)) {
                throw_spdlog_ex("Failed writing to systemd journal", errno);
            }
        }
    }
};

using systemd_namespace_sink_mt = systemd_namespace_sink<std::mutex>;
using systemd_namespace_sink_st = systemd_namespace_sink<details::null_mutex>;
}  // namespace sinks

template <typename Factory = synchronous_factory>
inline std::shared_ptr<logger> systemd_namespace_logger_mt(const std::string &logger_name,
                                                           const std::string &ident,
                                                           const std::string &name_space,
                                                           bool enable_formatting) {
    return Factory::template create<sinks::systemd_namespace_sink_mt>(
        logger_name, ident, name_space, enable_formatting);
}

template <typename Factory = synchronous_factory>
inline std::shared_ptr<logger> systemd_namespace_logger_st(const std::string &logger_name,
                                                           const std::string &ident,
                                                           const std::string &name_space,
                                                           bool enable_formatting) {
    return Factory::template create<sinks::systemd_namespace_sink_st>(
        logger_name, ident, name_space, enable_formatting);
}
SPDLOG_NAMESPACE_END
