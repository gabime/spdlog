#include "spdlite.h"
#include "spdlog/spdlog.h"

static spdlog::level::level_enum to_spdlog_level(spdlog::lite::level level)
{
    return static_cast<spdlog::level::level_enum>(level);
}

static spdlog::lite::level to_lite_level(spdlog::level::level_enum level)
{
    return static_cast<spdlog::lite::level>(level);
}

spdlog::lite::logger::logger(std::shared_ptr<spdlog::logger> impl)
{
    impl_ = std::move(impl);
}

bool spdlog::lite::logger::should_log(spdlog::lite::level level) const SPDLOG_NOEXCEPT
{
    auto spd_level = to_spdlog_level(level);
    return impl_->should_log(spd_level); // TODO avoid the call using local level member?
}

void spdlog::lite::logger::log(spdlog::lite::level lvl, const string_view_t &sv)
{
    auto spd_level = to_spdlog_level(lvl);
    impl_->log(spd_level, sv);
}

void spdlog::lite::logger::log_printf(spdlog::lite::level lvl, const char *format, va_list args)
{
    char buffer[500];
    auto size = vsnprintf(buffer, sizeof(buffer), format, args);
    if (size < 0)
    {
        size = snprintf(buffer, sizeof(buffer), "invalid format (%s)", format);
    }
    log(lvl, string_view_t{buffer, static_cast<size_t>(size)});
}

void spdlog::lite::logger::trace_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(lite::level::trace, format, args);
    va_end(args);
}

void spdlog::lite::logger::debug_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(lite::level::debug, format, args);
    va_end(args);
}

void spdlog::lite::logger::info_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(lite::level::info, format, args);
    va_end(args);
}

void spdlog::lite::logger::warn_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(lite::level::warn, format, args);
    va_end(args);
}

void spdlog::lite::logger::error_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(lite::level::err, format, args);
    va_end(args);
}

void spdlog::lite::logger::critical_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(lite::level::critical, format, args);
    va_end(args);
}

void spdlog::lite::logger::set_level(spdlog::lite::level level) noexcept
{
    auto spd_level = to_spdlog_level(level);
    impl_->set_level(spd_level);
}

spdlog::lite::level spdlog::lite::logger::level() const noexcept
{
    return to_lite_level(impl_->level());
}

std::string spdlog::lite::logger::name() const noexcept
{
    return impl_->name();
}

void spdlog::lite::logger::flush()
{
    impl_->flush();
}

void spdlog::lite::logger::flush_on(spdlog::lite::level level)
{
    auto spd_level = to_spdlog_level(level);
    impl_->flush_on(spd_level);
}

spdlog::lite::level spdlog::lite::logger::flush_level() const noexcept
{
    return to_lite_level(impl_->flush_level());
}

// pattern
void spdlog::lite::logger::set_pattern(std::string pattern) noexcept
{
    impl_->set_pattern(std::move(pattern));
}

spdlog::lite::logger spdlog::lite::logger::clone(std::string logger_name)
{
    return spdlog::lite::logger(impl_->clone(std::move(logger_name)));
}

void spdlog::lite::logger::log_formatted_(spdlog::lite::level lvl, const fmt::memory_buffer &formatted)
{
    auto spd_level = to_spdlog_level(lvl);
    impl_->log(spd_level, spdlog::details::fmt_helper::to_string_view(formatted));
}

spdlog::lite::logger &spdlog::lite::default_logger()
{
    static spdlog::lite::logger s_default(spdlog::default_logger());
    return s_default;
}
