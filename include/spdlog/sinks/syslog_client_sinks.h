// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/details/syslog_consts.h>
#include <spdlog/sinks/tcp_sink.h>

namespace spdlog {
namespace sinks {

/**
 * Sink that sends messages to remote syslog server over TCP
 */
template<typename Mutex>
class tcp_syslog_client_sink : public spdlog::sinks::tcp_sink<Mutex>
{
private:
    using syslog_facility_t = spdlog::details::syslog_consts::facility;
public:
    explicit tcp_syslog_client_sink(tcp_sink_config &&sink_config, syslog_facility_t syslog_facility) 
        : spdlog::sinks::tcp_sink<Mutex>{std::move(sink_config)}
        , syslog_levels_{
              /* spdlog::level::trace    */ spdlog::details::syslog_consts::SL_DEBUG,
              /* spdlog::level::debug    */ spdlog::details::syslog_consts::SL_DEBUG,
              /* spdlog::level::info     */ spdlog::details::syslog_consts::SL_INFO,
              /* spdlog::level::warn     */ spdlog::details::syslog_consts::SL_WARNING,
              /* spdlog::level::err      */ spdlog::details::syslog_consts::SL_ERR,
              /* spdlog::level::critical */ spdlog::details::syslog_consts::SL_CRIT,
              /* spdlog::level::off      */ spdlog::details::syslog_consts::SL_INFO}
        , syslog_facility_{syslog_facility}
        {

        }
protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);

        // https://datatracker.ietf.org/doc/html/rfc5424#section-6.2.1
        std::string pri{"<" + std::to_string((syslog_facility_ << 3) + syslog_prio_from_level(msg)) + ">"};

        memory_buf_t syslog_formatted;
        syslog_formatted.append(pri);
        syslog_formatted.append(formatted);

        if (!spdlog::sinks::tcp_sink<Mutex>::client_.is_connected())
        {
            spdlog::sinks::tcp_sink<Mutex>::client_.connect(
                spdlog::sinks::tcp_sink<Mutex>::config_.server_host, 
                spdlog::sinks::tcp_sink<Mutex>::config_.server_port);
        }
        spdlog::sinks::tcp_sink<Mutex>::client_.send(syslog_formatted.data(), syslog_formatted.size());
    }
private:
    using levels_array = std::array<int, 7>;

    levels_array      syslog_levels_;
    syslog_facility_t syslog_facility_;

    int syslog_prio_from_level(const details::log_msg &msg) const
    {
        return syslog_levels_.at(static_cast<levels_array::size_type>(msg.level));
    }
};

using tcp_syslog_client_sink_mt = tcp_syslog_client_sink<std::mutex>;
using tcp_syslog_client_sink_st = tcp_syslog_client_sink<spdlog::details::null_mutex>;

} // namespace sinks

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> tcp_syslog_client_logger_mt(const std::string &logger_name, 
    spdlog::sinks::tcp_sink_config sink_config,
    spdlog::details::syslog_consts::facility syslog_facility = spdlog::details::syslog_consts::facility::SF_USER)
{
    return Factory::template create<sinks::tcp_syslog_client_sink_mt>(logger_name, std::move(sink_config), syslog_facility);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> tcp_syslog_client_logger_st(const std::string &logger_name, 
    spdlog::sinks::tcp_sink_config sink_config,
    spdlog::details::syslog_consts::facility syslog_facility = spdlog::details::syslog_consts::facility::SF_USER)
{
    return Factory::template create<sinks::tcp_syslog_client_sink_st>(logger_name, std::move(sink_config), syslog_facility);
}

} // namespace spdlog