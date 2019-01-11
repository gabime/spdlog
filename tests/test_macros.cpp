/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */

#include "includes.h"

#if SPDLOG_ACTIVE_LEVEL != SPDLOG_LEVEL_DEBUG
#error "Invalid SPDLOG_ACTIVE_LEVEL in test. Should be SPDLOG_LEVEL_DEBUG"
#endif

TEST_CASE("debug and trace w/o format string", "[macros]]")
{

    prepare_logdir();
    std::string filename = "logs/simple_log";

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("logger", filename);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);

    SPDLOG_LOGGER_TRACE(logger, "Test message 1");
    SPDLOG_LOGGER_DEBUG(logger, "Test message 2");
    logger->flush();

    REQUIRE(ends_with(file_contents(filename), "Test message 2\n"));
    REQUIRE(count_lines(filename) == 1);

    spdlog::set_default_logger(logger);

    SPDLOG_TRACE("Test message 3");
    SPDLOG_DEBUG("Test message {}", 4);
    logger->flush();

    REQUIRE(ends_with(file_contents(filename), "Test message 4\n"));
    REQUIRE(count_lines(filename) == 2);
}

TEST_CASE("disable param evaluation", "[macros]")
{
    SPDLOG_TRACE("Test message {}", throw std::runtime_error("Should not be evaluated"));
}
