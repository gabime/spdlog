#include "includes.h"
#include "test_sink.h"

#include <spdlog/cfg/env.h>
#include <spdlog/cfg/argv.h>

TEST_CASE("env", "[loaders]")
{
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st >("l1");
    setenv("SPDLOG_LEVEL", "l1=warn", 1);
    spdlog::cfg::env::load_levels();
    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
    spdlog::drop(l1->name());
}

TEST_CASE("argv1", "[loaders]")
{
    const char* argv[] = {"ignore", "SPDLOG_LEVEL=l1=warn"};
    spdlog::cfg::argv::load_levels(2, argv);
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
    spdlog::drop(l1->name());
}

TEST_CASE("argv2", "[loaders]")
{
    const char* argv[] = {"ignore", "SPDLOG_LEVEL=l1=warn,trace"};
    spdlog::cfg::argv::load_levels(2, argv);
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::trace);
    spdlog::drop(l1->name());
}

TEST_CASE("argv3", "[loaders]")
{
    const char* argv[] = {"ignore", "SPDLOG_LEVEL="};
    spdlog::cfg::argv::load_levels(2, argv);
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::info);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
    spdlog::drop(l1->name());
}

TEST_CASE("argv4", "[loaders]")
{
    const char* argv[] = {"ignore", "SPDLOG_LEVEL=junk"};
    spdlog::cfg::argv::load_levels(2, argv);
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::info);
    spdlog::drop(l1->name());
}



