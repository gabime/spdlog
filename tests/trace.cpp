
#include "includes.h"
#include <iostream>

template<class T>
std::string log_deb(const T& what, spdlog::level::level_enum logger_level = spdlog::level::info)
{
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

    spdlog::logger oss_logger("oss", oss_sink);
    oss_logger.set_level(logger_level);
    oss_logger.set_pattern("%j:%#> %u");
    SPDLOG_DEBUG((&oss_logger), what);

    return oss.str().substr(0, oss.str().length() - spdlog::details::os::eol_size);
}


TEST_CASE("tracing", "[log_levels2]")
{
    REQUIRE(log_deb("Hello", spdlog::level::err) == "");
    REQUIRE(log_deb("Hello", spdlog::level::critical) == "");
    REQUIRE(log_deb("Hello", spdlog::level::info) == "");
    REQUIRE(log_deb("Hello", spdlog::level::debug) == "trace.cpp:14> log_deb");
    REQUIRE(log_deb("Hello", spdlog::level::trace) == "trace.cpp:14> log_deb");
}

