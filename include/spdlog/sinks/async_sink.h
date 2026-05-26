#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include "../details/async_log_msg.h"
#include "../details/err_helper.h"
#include "sink.h"

// async_sink is a sink that sends log messages to a dist_sink in a separate thread using a queue.
// The worker thread dequeues the messages and sends them to the dist_sink to perform the actual logging.
// Once the sink is destroyed, the worker thread empties the queue and exits.

SPDLOG_NAMESPACE_BEGIN
namespace details {  // forward declaration
template <typename T>
class mpmc_blocking_queue;
}  // namespace details
SPDLOG_NAMESPACE_END

SPDLOG_NAMESPACE_BEGIN
namespace sinks {

class SPDLOG_API async_sink final : public sink {
public:
    enum class overflow_policy : std::uint8_t {
        block,           // Block until the log message can be enqueued (default).
        overrun_oldest,  // Overrun the oldest message in the queue if full.
        discard_new      // Discard the log message if the queue is full
    };

    static constexpr size_t default_queue_size = 8192;
    static constexpr size_t max_queue_size = 250'000;

    struct config {
        size_t queue_size = default_queue_size;
        overflow_policy policy = overflow_policy::block;
        std::vector<std::shared_ptr<sink>> sinks;
        std::function<void()> on_thread_start = nullptr;
        std::function<void()> on_thread_stop = nullptr;
        err_handler custom_err_handler = nullptr;
    };

    explicit async_sink(config async_config);
    async_sink(const async_sink &) = delete;
    async_sink &operator=(const async_sink &) = delete;
    async_sink(async_sink &&) = delete;
    async_sink &operator=(async_sink &&) = delete;
    ~async_sink() override;

    // sink interface implementation
    void log(const details::log_msg &msg) override;
    void set_pattern(const std::string &pattern) override;
    void set_formatter(std::unique_ptr<formatter> sink_formatter) override;
    // enqueue flush request to the worker thread and return immediately(default)
    // if you need to wait for the actual flush to finish, call wait_all() after flush() or destruct the sink
    void flush() override;

    // non sink interface methods

    // wait until all logs were processed up to timeout millis and return false if timeout was reached
    [[nodiscard]] bool wait_all(std::chrono::milliseconds timeout) const;

    // wait until all logs were processed
    void wait_all() const;

    // return the number of overrun messages (effective only if policy is overrun_oldest)
    [[nodiscard]] size_t get_overrun_counter() const;

    // reset the overrun counter
    void reset_overrun_counter() const;

    // return the number of discarded messages (effective only if policy is discard_new)
    [[nodiscard]] size_t get_discard_counter() const;

    // reset the discard counter
    void reset_discard_counter() const;

    // return the current async_sink configuration
    [[nodiscard]] const config &get_config() const;

    // return the current async_sink configuration
    [[nodiscard]] config &get_config();

    // create an async_sink with one backend sink constructed with the given args.
    // example:
    // auto async_file = async_sink::with<spdlog::sinks::basic_file_sink_st>("mylog.txt");
    template <typename Sink, typename... SinkArgs>
    static std::shared_ptr<async_sink> with(SinkArgs &&...sink_args) {
        config cfg{};
        cfg.sinks.emplace_back(std::make_shared<Sink>(std::forward<SinkArgs>(sink_args)...));
        return std::make_shared<async_sink>(cfg);
    }

private:
    using async_log_msg = details::async_log_msg;
    using queue_t = details::mpmc_blocking_queue<async_log_msg>;

    void enqueue_message_(details::async_log_msg &&msg) const;
    void backend_loop_();
    void backend_log_(const details::log_msg &msg);
    void backend_flush_();

    config config_;
    std::unique_ptr<queue_t> q_;
    std::thread worker_thread_;
    details::err_helper err_helper_;
    std::atomic_bool terminate_worker_ = false;
};

}  // namespace sinks
SPDLOG_NAMESPACE_END
