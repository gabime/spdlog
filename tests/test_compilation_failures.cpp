#include "includes.h"

TEST_CASE("{fmt} FMT_STRING functionality preserved (negative test)", "[fmt][fail][fail compilation]")
{
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

    spdlog::set_default_logger(std::make_shared<spdlog::logger>("oss", oss_sink));
    spdlog::default_logger()->set_level(spdlog::level::trace);

    spdlog::info(FMT_STRING("The best part of {{fmt}} is the compile time checking: {:d}"), "I shouldn't compile");
    // This should never be able to compile, so running is a failure.
    FAIL("This test case isn't meant to compile, let alone run.");
}
