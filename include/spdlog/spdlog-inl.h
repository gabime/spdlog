// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#    include <spdlog/spdlog.h>
#endif

#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace spdlog {

SPDLOG_INLINE std::shared_ptr<spdlog::logger> &default_logger()
{
    static auto s_default_logger = stdout_color_mt("");
    return s_default_logger;
}

SPDLOG_INLINE void set_default_logger(std::shared_ptr<spdlog::logger> new_logger)
{
    default_logger() = std::move(new_logger);
}

} // namespace spdlog
