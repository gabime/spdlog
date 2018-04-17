#include "includes.h"

template<class T>
std::string log_info(const T &what, spdlog::level::level_enum logger_level = spdlog::level::info)
{

    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

    spdlog::logger oss_logger("oss", oss_sink);
    oss_logger.set_level(logger_level);
    oss_logger.set_pattern("%v");
    oss_logger.info(what);

    return oss.str().substr(0, oss.str().length() - strlen(spdlog::details::os::default_eol));
}

TEST_CASE("basic_logging ", "[basic_logging]")
{
    // const char
    REQUIRE(log_info("Hello") == "Hello");
    REQUIRE(log_info("") == "");

    // std::string
    REQUIRE(log_info(std::string("Hello")) == "Hello");
    REQUIRE(log_info(std::string()) == std::string());

    // Numbers
    REQUIRE(log_info(5) == "5");
    REQUIRE(log_info(5.6) == "5.6");

    // User defined class
    // REQUIRE(log_info(some_logged_class("some_val")) == "some_val");
}

TEST_CASE("log_levels", "[log_levels]")
{
    REQUIRE(log_info("Hello", spdlog::level::err) == "");
    REQUIRE(log_info("Hello", spdlog::level::critical) == "");
    REQUIRE(log_info("Hello", spdlog::level::info) == "Hello");
    REQUIRE(log_info("Hello", spdlog::level::debug) == "Hello");
    REQUIRE(log_info("Hello", spdlog::level::trace) == "Hello");
}

TEST_CASE("to_str", "[convert_to_str]")
{
    REQUIRE(std::string(spdlog::level::to_str(spdlog::level::trace)) == "trace");
    REQUIRE(std::string(spdlog::level::to_str(spdlog::level::debug)) == "debug");
    REQUIRE(std::string(spdlog::level::to_str(spdlog::level::info)) == "info");
    REQUIRE(std::string(spdlog::level::to_str(spdlog::level::warn)) == "warning");
    REQUIRE(std::string(spdlog::level::to_str(spdlog::level::err)) == "error");
    REQUIRE(std::string(spdlog::level::to_str(spdlog::level::critical)) == "critical");
    REQUIRE(std::string(spdlog::level::to_str(spdlog::level::off)) == "off");
}

TEST_CASE("to_short_str", "[convert_to_short_str]")
{
    REQUIRE(std::string(spdlog::level::to_short_str(spdlog::level::trace)) == "T");
    REQUIRE(std::string(spdlog::level::to_short_str(spdlog::level::debug)) == "D");
    REQUIRE(std::string(spdlog::level::to_short_str(spdlog::level::info)) == "I");
    REQUIRE(std::string(spdlog::level::to_short_str(spdlog::level::warn)) == "W");
    REQUIRE(std::string(spdlog::level::to_short_str(spdlog::level::err)) == "E");
    REQUIRE(std::string(spdlog::level::to_short_str(spdlog::level::critical)) == "C");
    REQUIRE(std::string(spdlog::level::to_short_str(spdlog::level::off)) == "O");
}

TEST_CASE("to_level_enum", "[convert_to_level_enum]")
{
    REQUIRE(spdlog::level::from_str("trace") == spdlog::level::trace);
    REQUIRE(spdlog::level::from_str("debug") == spdlog::level::debug);
    REQUIRE(spdlog::level::from_str("info") == spdlog::level::info);
    REQUIRE(spdlog::level::from_str("warning") == spdlog::level::warn);
    REQUIRE(spdlog::level::from_str("error") == spdlog::level::err);
    REQUIRE(spdlog::level::from_str("critical") == spdlog::level::critical);
    REQUIRE(spdlog::level::from_str("off") == spdlog::level::off);
    REQUIRE(spdlog::level::from_str("null") == spdlog::level::off);
}
