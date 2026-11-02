#include "includes.h"
#include "test_sink.h"
#include <chrono>

TEST_CASE("SPD_LOGGER_CONDITIONAL TRUEITY CASE", "[simple logger cond1]")
{
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_st>();
    int errors = 5;
    auto logger = std::make_shared<spdlog::logger>("Luis's logger", test_sink);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);
    SPDLOG_LOGGER_CONDITIONAL(logger, errors > 0, spdlog::level::err, "These are the errors: {}", errors);
    REQUIRE(test_sink->msg_counter() == 1);
    REQUIRE(test_sink->lines()[0] == "These are the errors: 5");
}

TEST_CASE("SPD_LOGGER CONDITIONAL FALSITY CASE", "[simple logger cond2]")
{
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_st>();
    int errors = 5;
    auto logger = std::make_shared<spdlog::logger>("Luis's logger", test_sink);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);
    SPDLOG_LOGGER_CONDITIONAL(logger, errors > 20,  spdlog::level::err, "These are the errors: {}", errors);
    REQUIRE(test_sink->msg_counter() == 0);
}

TEST_CASE("SPD_LOGGER_EVERY_N BASIC FOR CASE", "[log every N 100 case]")
{
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_st>();
    auto logger = std::make_shared<spdlog::logger>("Luis's logger", test_sink);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);
    for(int i = 0; i < 100; i++)
    {
        SPDLOG_LOGGER_EVERY_N(logger, spdlog::level::err, 10, "Activate log at: {}", i);
    }
    for(int i = 0; i < 100; i++)
    {
        SPDLOG_LOGGER_EVERY_N(logger, spdlog::level::err, 10, "Activate log at: {}", i);
    }
    REQUIRE(test_sink->msg_counter() == 20);
    std::string temp;
    for(int i = 0; i < 10; i++)
    {
        temp = "Activate log at: "+ std::to_string(i*10);
        REQUIRE(test_sink->lines()[i] == temp);
    }
    for(int i = 0; i < 10; i++)
    {
        temp = "Activate log at: "+ std::to_string(i*10);
        REQUIRE(test_sink->lines()[9+i+1] == temp);
    }
}

TEST_CASE("SPDLOG_LOGGER_THROTTLE check" , "[log 0 sec test]")
{
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_st>();
    auto logger = std::make_shared<spdlog::logger>("Luis's logger", test_sink);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);
    for(int i = 0; i < 100; i++)
    {
        SPDLOG_LOGGER_THROTTLE(logger, spdlog::level::err, 1.0, "Log active once");
    }
    REQUIRE(test_sink->msg_counter() == 1);
    REQUIRE(test_sink->lines()[0] ==  "Log active once");
}

TEST_CASE("SPDLOG_LOGGER_THROTTLE check" , "[log 2 sec test]")
{
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_st>();
    auto logger = std::make_shared<spdlog::logger>("Luis's logger", test_sink);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);
    for(int i = 0; i < 99999999; i++)
    {
        SPDLOG_LOGGER_THROTTLE(logger, spdlog::level::err, 1.0, "Log active once");
    }
    REQUIRE(test_sink->msg_counter() == 3);
}
