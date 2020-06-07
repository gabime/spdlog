// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <spdlog/spdlog.h>
#include <spdlog/common.h>
#include <spdlog/pattern_formatter.h>

namespace spdlog {

void initialize_logger(std::shared_ptr<logger> logger)
{
    details::registry::instance().initialize_logger(std::move(logger));
}

std::shared_ptr<logger> get(const std::string &name)
{
    return details::registry::instance().get(name);
}

void set_formatter(std::unique_ptr<spdlog::formatter> formatter)
{
    details::registry::instance().set_formatter(std::move(formatter));
}

void set_pattern(std::string pattern, pattern_time_type time_type)
{
    set_formatter(std::unique_ptr<spdlog::formatter>(new pattern_formatter(std::move(pattern), time_type)));
}

void set_level(level::level_enum log_level)
{
    details::registry::instance().set_level(log_level);
}

void flush_on(level::level_enum log_level)
{
    details::registry::instance().flush_on(log_level);
}

void flush_every(std::chrono::seconds interval)
{
    details::registry::instance().flush_every(interval);
}

void set_error_handler(void (*handler)(const std::string &msg))
{
    details::registry::instance().set_error_handler(handler);
}

void register_logger(std::shared_ptr<logger> logger)
{
    details::registry::instance().register_logger(std::move(logger));
}

void apply_all(const std::function<void(std::shared_ptr<logger>)> &fun)
{
    details::registry::instance().apply_all(fun);
}

void drop(const std::string &name)
{
    details::registry::instance().drop(name);
}

void drop_all()
{
    details::registry::instance().drop_all();
}

void shutdown()
{
    details::registry::instance().shutdown();
}

void set_automatic_registration(bool automatic_registration)
{
    details::registry::instance().set_automatic_registration(automatic_registration);
}

std::shared_ptr<spdlog::logger> default_logger()
{
    return details::registry::instance().default_logger();
}

spdlog::logger *default_logger_raw()
{
    return details::registry::instance().get_default_raw();
}

void set_default_logger(std::shared_ptr<spdlog::logger> default_logger)
{
    details::registry::instance().set_default_logger(std::move(default_logger));
}

} // namespace spdlog

// template instantiate
template SPDLOG_API spdlog::logger::logger(std::string name, sinks_init_list::iterator begin, sinks_init_list::iterator end);
