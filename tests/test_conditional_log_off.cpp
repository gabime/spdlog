/*
 * This content is released under the MIT License as specified in
 * https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */

// Separate translation unit compiled with SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_OFF.
// Verifies the conditional macros are compiled out and never evaluate their
// arguments: the counter must stay at 0 and nothing is written to disk.
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_OFF

#include <catch2/catch_all.hpp>

#include "utils.h"

#include "spdlog/spdlog.h"
#include "spdlog/conditional_log.h"
#include "spdlog/sinks/basic_file_sink.h"

TEST_CASE("conditional macros do not evaluate arguments when the active level is off",
          "[conditional_off]") {
    prepare_logdir();

    spdlog::filename_t filename = SPDLOG_FILENAME_T("test_logs/conditional_off_log");
    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("off_logger", filename);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);

    int evaluations = 0;

    // Base macros with explicit level
    SPDLOG_LOGGER_EVERY_N(logger, spdlog::level::info, 1, "every {}", ++evaluations);
    SPDLOG_LOGGER_THROTTLE(logger, spdlog::level::info, std::chrono::seconds(0),
                           "throttle {}", ++evaluations);

    // Per-level macros
    SPDLOG_LOGGER_INFO_EVERY_N(logger, 1, "info_every {}", ++evaluations);
    SPDLOG_LOGGER_INFO_THROTTLE(logger, std::chrono::seconds(0), "info_throttle {}", ++evaluations);

    // Default logger per-level macros
    SPDLOG_INFO_EVERY_N(1, "default_info_every {}", ++evaluations);
    SPDLOG_INFO_THROTTLE(std::chrono::seconds(0), "default_info_throttle {}", ++evaluations);

    logger->flush();
    REQUIRE(evaluations == 0);
    REQUIRE(count_lines("test_logs/conditional_off_log") == 0);

    spdlog::drop("off_logger");
}
