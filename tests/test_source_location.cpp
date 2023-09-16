#define SPDLOG_EMIT_SOURCE_LOCATION
#include "includes.h"
#include "test_sink.h"

#if defined(SPDLOG_STD_SOURCE_LOCATION) || defined(SPDLOG_EXPERIMENTAL_SOURCE_LOCATION)

using spdlog::details::os::default_eol;

TEST_CASE("test_source_location", "[source_location]")
{
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_st>(oss);
    spdlog::logger oss_logger("oss", oss_sink);
    oss_logger.set_pattern("%s:%# %v");

    oss_logger.info("Hello {}", "source location");
    REQUIRE(oss.str() == std::string("test_source_location.cpp:16 Hello source location") + default_eol);
}

#endif

