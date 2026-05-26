// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/spdlog.h"

#include <cassert>
#include <memory>

#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/pattern_formatter.h"
#include "spdlog/sinks/stdout_color_sinks.h"

SPDLOG_NAMESPACE_BEGIN

#ifndef SPDLOG_DISABLE_GLOBAL_LOGGER
static std::shared_ptr<logger> s_logger = std::make_shared<logger>("", std::make_shared<sinks::stdout_color_sink_mt>());
#else
static std::shared_ptr<logger> s_logger = nullptr;
#endif

std::shared_ptr<logger> global_logger() { return s_logger; }

void set_global_logger(std::shared_ptr<logger> global_logger) { s_logger = std::move(global_logger); }

logger *global_logger_raw() noexcept { return s_logger.get(); }

void set_formatter(std::unique_ptr<formatter> formatter) { global_logger()->set_formatter(std::move(formatter)); }

void set_pattern(std::string pattern, pattern_time_type time_type) {
    set_formatter(std::make_unique<pattern_formatter>(std::move(pattern), time_type));
}

level get_level() { return global_logger()->log_level(); }

bool should_log(level log_level) { return global_logger()->should_log(log_level); }

void set_level(level level) { global_logger()->set_level(level); }

void flush_on(level level) { global_logger()->flush_on(level); }

void set_error_handler(void (*handler)(const std::string &msg)) { global_logger()->set_error_handler(handler); }

void shutdown() { s_logger.reset(); }

SPDLOG_NAMESPACE_END
