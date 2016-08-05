
#include "includes.h"

template<class T>
std::string log_info(const T& what, spdlog::level::level_enum logger_level = spdlog::level::info)
{

    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

    spdlog::logger oss_logger("oss", oss_sink);
    oss_logger.set_level(logger_level);
    oss_logger.set_pattern("%v");
    oss_logger.info(what);

    return oss.str().substr(0, oss.str().length() - spdlog::details::os::eol_size);
}






TEST_CASE("basic_logging ", "[basic_logging]")
{
    //const char
    REQUIRE(log_info("Hello") == "Hello");
    REQUIRE(log_info("") == "");

    //std::string
    REQUIRE(log_info(std::string("Hello")) == "Hello");
    REQUIRE(log_info(std::string()) == std::string());

    //Numbers
    REQUIRE(log_info(5) == "5");
    REQUIRE(log_info(5.6) == "5.6");

    //User defined class
    //REQUIRE(log_info(some_logged_class("some_val")) == "some_val");
}


TEST_CASE("log_levels", "[log_levels]")
{
    REQUIRE(log_info("Hello", spdlog::level::err) == "");
    REQUIRE(log_info("Hello", spdlog::level::critical) == "");
    REQUIRE(log_info("Hello", spdlog::level::info) == "Hello");
    REQUIRE(log_info("Hello", spdlog::level::debug) == "Hello");
    REQUIRE(log_info("Hello", spdlog::level::trace) == "Hello");
}






