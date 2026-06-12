// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

// Grafana Loki sink - sends log messages to Loki via the HTTP push API.
// Requires nlohmann/json (https://github.com/nlohmann/json) and
// cpp-httplib (https://github.com/yhirose/cpp-httplib).
// Docs: https://grafana.com/docs/loki/latest/reference/loki-http-api/#ingest-logs
//
// The default pattern is "%v" (message text only) because Loki stores the timestamp separately and
// (by default) the log level is sent as a "level" stream label. Override with set_pattern() to include
// additional context in the log line (e.g. "%t %v" to prepend the thread id).
//
// For production use, prefer loki_logger_async_mt: sink_it_() makes a blocking
// HTTP call and should not run on the caller's thread.
//
// Limitations:
// - Sends one log entry per HTTP request (no batching).
// - No TLS/auth header support (e.g. Grafana Cloud / auth-protected setups).

#include <spdlog/async.h>
#include <spdlog/common.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/base_sink.h>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>

SPDLOG_NAMESPACE_BEGIN
namespace sinks {

struct loki_sink_config {
    std::string server_host;
    int server_port;
    std::unordered_map<std::string, std::string> labels;
    bool add_level_label = true;
    // 0 = use cpp-httplib defaults (300s connect, 300s read/write). Set explicitly
    // when using the synchronous factory to bound blocking time in sink_it_().
    int timeout_seconds = 0;

    loki_sink_config(std::string host, int port)
        : server_host{std::move(host)},
          server_port{port} {}
};

template <typename Mutex>
class loki_sink : public base_sink<Mutex> {
public:
    explicit loki_sink(loki_sink_config config)
        : config_{std::move(config)},
          client_(config_.server_host, config_.server_port),
          labels_json_(config_.labels) {
        // Timestamp and level go into Loki structured fields, so default to message text only.
        // Users can call set_pattern() to include additional context (e.g. thread id).
        base_sink<Mutex>::set_formatter_(
            details::make_unique<pattern_formatter>("%v", pattern_time_type::local, ""));
        client_.set_keep_alive(true);
        if (config_.timeout_seconds > 0) {
            client_.set_connection_timeout(config_.timeout_seconds);
            client_.set_read_timeout(config_.timeout_seconds);
            client_.set_write_timeout(config_.timeout_seconds);
        }
    }

    ~loki_sink() override = default;
    loki_sink(const loki_sink &) = delete;
    loki_sink &operator=(const loki_sink &) = delete;

protected:
    void sink_it_(const details::log_msg &msg) override {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        auto body = build_json_(msg, std::string(formatted.data(), formatted.size()));
        auto res = client_.Post("/loki/api/v1/push", body, "application/json");
        if (!res)
            throw_spdlog_ex("loki_sink: connection failed");
        if (res->status != 204)
            throw_spdlog_ex("loki_sink: HTTP " + std::to_string(res->status) + " " + res->reason +
                            (res->body.empty() ? "" : ": " + res->body));
    }

    void flush_() override {}

    void set_pattern_(const std::string &pattern) override {
        this->set_formatter_(details::make_unique<pattern_formatter>(pattern, pattern_time_type::local, ""));
    }

private:
    std::string build_json_(const details::log_msg &msg, const std::string &line) const {
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      msg.time.time_since_epoch())
                      .count();
        if (ns < 0) ns = 0;

        nlohmann::json labels = labels_json_;
        if (config_.add_level_label && !labels.contains("level")) {
            auto sv = level::to_string_view(msg.level);
            labels["level"] = std::string(sv.data(), sv.size());
        }

        return nlohmann::json{{"streams", {{
            {"stream", labels},
            {"values", nlohmann::json::array({nlohmann::json::array({std::to_string(ns), line})})}
        }}}}.dump();
    }

    loki_sink_config config_;
    httplib::Client client_;
    nlohmann::json labels_json_;
};

using loki_sink_mt = loki_sink<std::mutex>;
using loki_sink_st = loki_sink<details::null_mutex>;

}  // namespace sinks

template <typename Factory = synchronous_factory>
inline std::shared_ptr<logger> loki_logger_mt(const std::string &logger_name,
                                               sinks::loki_sink_config config) {
    return Factory::template create<sinks::loki_sink_mt>(logger_name, std::move(config));
}

template <typename Factory = synchronous_factory>
inline std::shared_ptr<logger> loki_logger_st(const std::string &logger_name,
                                               sinks::loki_sink_config config) {
    return Factory::template create<sinks::loki_sink_st>(logger_name, std::move(config));
}

template <typename Factory = async_factory>
inline std::shared_ptr<logger> loki_logger_async_mt(const std::string &logger_name,
                                                     sinks::loki_sink_config config) {
    return Factory::template create<sinks::loki_sink_mt>(logger_name, std::move(config));
}

template <typename Factory = async_factory>
inline std::shared_ptr<logger> loki_logger_async_st(const std::string &logger_name,
                                                     sinks::loki_sink_config config) {
    return Factory::template create<sinks::loki_sink_st>(logger_name, std::move(config));
}

SPDLOG_NAMESPACE_END
