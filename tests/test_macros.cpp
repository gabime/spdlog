/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */

#include "includes.h"

#if SPDLOG_ACTIVE_LEVEL != SPDLOG_LEVEL_DEBUG
#    error "Invalid SPDLOG_ACTIVE_LEVEL in test. Should be SPDLOG_LEVEL_DEBUG"
#endif

#define TEST_FILENAME "test_logs/simple_log"

TEST_CASE("debug and trace w/o format string", "[macros]")
{

    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T(TEST_FILENAME);

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("logger", filename);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);

    SPDLOG_LOGGER_TRACE(logger, "Test message 1");
    SPDLOG_LOGGER_DEBUG(logger, "Test message 2");
    logger->flush();

    using spdlog::details::os::default_eol;
    REQUIRE(ends_with(file_contents(TEST_FILENAME), spdlog::fmt_lib::format("Test message 2{}", default_eol)));
    REQUIRE(count_lines(TEST_FILENAME) == 1);

    auto orig_default_logger = spdlog::default_logger();
    spdlog::set_default_logger(logger);

    SPDLOG_TRACE("Test message 3");
    SPDLOG_DEBUG("Test message {}", 4);
    logger->flush();

    require_message_count(TEST_FILENAME, 2);
    REQUIRE(ends_with(file_contents(TEST_FILENAME), spdlog::fmt_lib::format("Test message 4{}", default_eol)));
    spdlog::set_default_logger(std::move(orig_default_logger));
}

TEST_CASE("disable param evaluation", "[macros]")
{
    SPDLOG_TRACE("Test message {}", throw std::runtime_error("Should not be evaluated"));
}

TEST_CASE("pass logger pointer", "[macros]")
{
    auto logger = spdlog::create<spdlog::sinks::null_sink_mt>("refmacro");
    auto &ref = *logger;
    SPDLOG_LOGGER_TRACE(&ref, "Test message 1");
    SPDLOG_LOGGER_DEBUG(&ref, "Test message 2");
}
