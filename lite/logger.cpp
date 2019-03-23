#include "logger.h"
#include "spdlog/spdlog.h"
#include "spdlog/logger.h"

spdlog::lite::logger::logger(std::shared_ptr<spdlog::logger> impl)
{
    impl_ = std::move(impl);
}


bool spdlog::lite::logger::should_log(spdlog::lite::level lvl) const SPDLOG_NOEXCEPT
{
    auto spd_level = static_cast<spdlog::level::level_enum >(lvl);
    return impl_->should_log(spd_level);//TODO level
}

void spdlog::lite::logger::log_formatted_(spdlog::lite::level lvl, const fmt::memory_buffer &formatted)
{
    auto spd_level = static_cast<spdlog::level::level_enum >(lvl);
    impl_->log(spd_level, spdlog::details::fmt_helper::to_string_view(formatted)); //TODO and source_loc
}