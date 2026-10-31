#include "includes.h"
#include "test_sink.h"

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
