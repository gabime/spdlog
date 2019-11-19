// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

// Loggers registry of unique name->logger pointer
// An attempt to create a logger with an already existing name will result with spdlog_ex exception.
// If user requests a non existing logger, nullptr will be returned
// This class is thread safe

#include <spdlog/common.h>
#include <spdlog/async_logger.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

namespace spdlog {
class logger;
class async_logger;

namespace details {
class thread_pool;
class periodic_worker;

static const size_t default_async_q_size = 8192;

class registry
{
public:
	// Default constructor
	registry();
	~registry();

    registry(const registry &) = delete;
    registry &operator=(const registry &) = delete;

    void register_logger(std::shared_ptr<logger> new_logger);
    void initialize_logger(std::shared_ptr<logger> new_logger);
    std::shared_ptr<logger> get(const std::string &logger_name);
    std::shared_ptr<logger> default_logger();

    // Return raw ptr to the default logger.
    // To be used directly by the spdlog default api (e.g. spdlog::info)
    // This make the default API faster, but cannot be used concurrently with set_default_logger().
    // e.g do not call set_default_logger() from one thread while calling spdlog::info() from another.
    logger *get_default_raw();

    // set default logger.
    // default logger is stored in default_logger_ (for faster retrieval) and in the loggers_ map.
    void set_default_logger(std::shared_ptr<logger> new_default_logger);

    void set_tp(std::shared_ptr<thread_pool> tp);

    std::shared_ptr<thread_pool> get_tp();

    // Set global formatter. Each sink in each logger will get a clone of this object
    void set_formatter(std::unique_ptr<formatter> formatter);

    void enable_backtrace(size_t n_messages);

    void disable_backtrace();

    void set_level(level::level_enum log_level);

    void flush_on(level::level_enum log_level);

    void flush_every(std::chrono::seconds interval);

    void set_error_handler(void (*handler)(const std::string &msg));

    void apply_all(const std::function<void(const std::shared_ptr<logger>)> &fun);

    void flush_all();

    void drop(const std::string &logger_name);

    void drop_all();

    // clean all resources and threads started by the registry
    void shutdown();

    std::recursive_mutex &tp_mutex();

    void set_automatic_registration(bool automatic_registration);

	// Factory function to create a new logger and register it in this registry
	template<typename Sink, typename... SinkArgs>
	std::shared_ptr<spdlog::logger> create(std::string logger_name, SinkArgs&& ... args)
	{
		auto sink = std::make_shared<Sink>(std::forward<SinkArgs>(args)...);
		auto new_logger = std::make_shared<spdlog::logger>(std::move(logger_name), std::move(sink));
		this->initialize_logger(new_logger);
		return new_logger;
	}

    template<typename Sink, typename... SinkArgs>
    std::shared_ptr<async_logger> create_async(std::string logger_name, 
                                               async_overflow_policy OverflowPolicy,
                                               SinkArgs &&... args)
    {
        // create global thread pool if not already exists..

        std::lock_guard<std::recursive_mutex> tp_lock(tp_mutex_);
        if (tp_ == nullptr)
        {
            tp_ = std::make_shared<details::thread_pool>(details::default_async_q_size, 1);
        }

        auto sink = std::make_shared<Sink>(std::forward<SinkArgs>(args)...);
        auto new_logger = std::make_shared<async_logger>(std::move(logger_name), std::move(sink), std::move(tp_), OverflowPolicy);
        this->initialize_logger(new_logger);
        return new_logger;
    }

	// The unique global instance of the registry, usefult to access it
	// from the global scope. NOTE It doesn't make it a singleton
    static registry &instance();

private:
    void throw_if_exists_(const std::string &logger_name);
    void register_logger_(std::shared_ptr<logger> new_logger);
    std::mutex logger_map_mutex_, flusher_mutex_;
    std::recursive_mutex tp_mutex_;
    std::unordered_map<std::string, std::shared_ptr<logger>> loggers_;
    std::unique_ptr<formatter> formatter_;
    level::level_enum level_ = level::info;
    level::level_enum flush_level_ = level::off;
    void (*err_handler_)(const std::string &msg);
    std::shared_ptr<thread_pool> tp_;
    std::unique_ptr<periodic_worker> periodic_flusher_;
    std::shared_ptr<logger> default_logger_;
    bool automatic_registration_ = true;
    size_t backtrace_n_messages_ = 0;
};

} // namespace details
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "registry-inl.h"
#endif
