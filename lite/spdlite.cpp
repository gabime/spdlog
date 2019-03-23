#include "spdlite.h"
#include "spdlog/spdlog.h"
#include "spdlog/logger.h"

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
    return impl_->should_log(spd_level); // TODO level
}

void spdlog::lite::logger::log_formatted_(spdlog::lite::level level, const fmt::memory_buffer &formatted)
{
    auto spd_level = to_spdlog_level(level);
    impl_->log(spd_level, spdlog::details::fmt_helper::to_string_view(formatted));
}

void spdlog::lite::logger::log_formatted_src(const spdlog::lite::src_loc &src, spdlog::lite::level lvl, const fmt::memory_buffer &formatted)
{
    auto spd_level = to_spdlog_level(lvl);
    spdlog::source_loc source_loc{src.filename, src.line, src.funcname};
    impl_->log(source_loc, spd_level, spdlog::details::fmt_helper::to_string_view(formatted));
}

void spdlog::lite::logger::set_level(spdlog::lite::level level)
{
    auto spd_level = to_spdlog_level(level);
    impl_->set_level(spd_level);
}

spdlog::lite::level spdlog::lite::logger::get_level() const
{
    return to_lite_level(impl_->level());
}

std::string spdlog::lite::logger::name() const
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

spdlog::lite::level spdlog::lite::logger::flush_level() const
{
    return to_lite_level(impl_->flush_level());
}

// pattern
void spdlog::lite::logger::set_pattern(std::string pattern)
{
    impl_->set_pattern(std::move(pattern));
}

spdlog::lite::logger &spdlog::lite::default_logger()
{
    static spdlog::lite::logger s_default(spdlog::default_logger());
    return s_default;
}
