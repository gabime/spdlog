/*
* This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
*/

#include "includes.h"

TEST_CASE("debug and trace w/o format string", "[macros]]")
{
    prepare_logdir();
    std::string filename = "logs/simple_log";

    auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("logger", filename);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);

    SPDLOG_TRACE(logger, "Test message 1");
    //SPDLOG_DEBUG(logger, "Test message 2");
    SPDLOG_DEBUG(logger, "Test message 2");
    logger->flush();

    REQUIRE(ends_with(file_contents(filename), "Test message 2\n"));
    REQUIRE(count_lines(filename) == 2);
}


TEST_CASE("debug and trace with format strings", "[macros]]")
{
    prepare_logdir();
    std::string filename = "logs/simple_log";

    auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("logger", filename);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);

#if !defined(SPDLOG_FMT_PRINTF)
    SPDLOG_TRACE(logger, "Test message {}", 1);
    //SPDLOG_DEBUG(logger, "Test message 2");
    SPDLOG_DEBUG(logger, "Test message {}", 222);
#else
    SPDLOG_TRACE(logger, "Test message %d", 1);
    //SPDLOG_DEBUG(logger, "Test message 2");
    SPDLOG_DEBUG(logger, "Test message %d", 222);
#endif

    logger->flush();

    REQUIRE(ends_with(file_contents(filename), "Test message 222\n"));
    REQUIRE(count_lines(filename) == 2);
}

