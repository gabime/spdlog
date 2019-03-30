#include "spdlite.h"
#include "spdlog/spdlog.h"

static spdlog::level::level_enum to_spdlog_level(spdlite::level level)
{
    return static_cast<spdlog::level::level_enum>(level);
}

static spdlite::level to_lite_level(spdlog::level::level_enum level)
{
    return static_cast<spdlite::level>(level);
}

spdlite::logger::logger(std::shared_ptr<spdlog::logger> impl)
{
    impl_ = std::move(impl);
}

bool spdlite::logger::should_log(spdlite::level level) const SPDLOG_NOEXCEPT
{
    auto spd_level = to_spdlog_level(level);
    return impl_->should_log(spd_level); // TODO avoid the call using local level member?
}

void spdlite::logger::log(spdlite::level lvl, const string_view_t &sv)
{
    auto spd_level = to_spdlog_level(lvl);
    impl_->log(spd_level, sv);
}

void spdlite::logger::log_printf(spdlite::level lvl, const char *format, va_list args)
{
    char buffer[500];
    auto size = vsnprintf(buffer, sizeof(buffer), format, args);
    if (size < 0)
    {
        size = snprintf(buffer, sizeof(buffer), "invalid format (%s)", format);
    }
    log(lvl, string_view_t{buffer, static_cast<size_t>(size)});
}

void spdlite::logger::trace_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(spdlite::level::trace, format, args);
    va_end(args);
}

void spdlite::logger::debug_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(spdlite::level::debug, format, args);
    va_end(args);
}

void spdlite::logger::info_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(spdlite::level::info, format, args);
    va_end(args);
}

void spdlite::logger::warn_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(spdlite::level::warn, format, args);
    va_end(args);
}

void spdlite::logger::error_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(spdlite::level::err, format, args);
    va_end(args);
}

void spdlite::logger::critical_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(spdlite::level::critical, format, args);
    va_end(args);
}

void spdlite::logger::set_level(spdlite::level level) noexcept
{
    auto spd_level = to_spdlog_level(level);
    impl_->set_level(spd_level);
}

spdlite::level spdlite::logger::level() const noexcept
{
    return to_lite_level(impl_->level());
}

std::string spdlite::logger::name() const noexcept
{
    return impl_->name();
}

void spdlite::logger::flush()
{
    impl_->flush();
}

void spdlite::logger::flush_on(spdlite::level level)
{
    auto spd_level = to_spdlog_level(level);
    impl_->flush_on(spd_level);
}

spdlite::level spdlite::logger::flush_level() const noexcept
{
    return to_lite_level(impl_->flush_level());
}

// pattern
void spdlite::logger::set_pattern(std::string pattern) noexcept
{
    impl_->set_pattern(std::move(pattern));
}

spdlite::logger spdlite::logger::clone(std::string logger_name)
{
    return spdlite::logger(impl_->clone(std::move(logger_name)));
}

void spdlite::logger::log_formatted_(spdlite::level lvl, const fmt::memory_buffer &formatted)
{
    auto spd_level = to_spdlog_level(lvl);
    impl_->log(spd_level, spdlog::details::fmt_helper::to_string_view(formatted));
}

spdlite::logger &spdlite::default_logger()
{
    static spdlite::logger s_default(spdlog::default_logger());
    return s_default;
}
