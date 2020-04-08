#include "includes.h"
#include "test_sink.h"

#include <spdlog/cfg/env.h>
#include <spdlog/cfg/argv.h>

using spdlog::cfg::load_argv_levels;
using spdlog::cfg::load_env_levels;
using spdlog::sinks::test_sink_st;

TEST_CASE("env", "[cfg]")
{
    spdlog::drop("l1");
    auto l1 = spdlog::create<test_sink_st>("l1");
#ifdef CATCH_PLATFORM_WINDOWS
    _putenv_s("SPDLOG_LEVEL", "l1=warn");
#else
    setenv("SPDLOG_LEVEL", "l1=warn", 1);
#endif
    load_env_levels();
    REQUIRE(l1->level() == spdlog::level::warn);
    spdlog::set_default_logger(spdlog::create<test_sink_st>("cfg-default"));
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
}

TEST_CASE("argv1", "[cfg]")
{
    spdlog::drop("l1");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=l1=warn"};
    load_argv_levels(2, argv);
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
}

TEST_CASE("argv2", "[cfg]")
{
    spdlog::drop("l1");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=l1=warn,trace"};
    load_argv_levels(2, argv);
    auto l1 = spdlog::create<test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::trace);
    spdlog::set_level(spdlog::level::info);
}

TEST_CASE("argv3", "[cfg]")
{
    spdlog::drop("l1");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL="};
    load_argv_levels(2, argv);
    auto l1 = spdlog::create<test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::info);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
}

TEST_CASE("argv4", "[cfg]")
{
    spdlog::drop("l1");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=junk"};
    load_argv_levels(2, argv);
    auto l1 = spdlog::create<test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::info);
}

TEST_CASE("argv5", "[cfg]")
{
    spdlog::drop("l1");
    const char *argv[] = {"ignore", "ignore", "SPDLOG_LEVEL=l1=warn,trace"};
    load_argv_levels(3, argv);
    auto l1 = spdlog::create<test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::trace);
    spdlog::set_level(spdlog::level::info);
}

TEST_CASE("argv6", "[cfg]")
{
    spdlog::set_level(spdlog::level::err);
    const char *argv[] = {""};
    load_argv_levels(1, argv);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::err);
    spdlog::set_level(spdlog::level::info);
}

TEST_CASE("argv7", "[cfg]")
{
    spdlog::set_level(spdlog::level::err);
    const char *argv[] = {""};
    load_argv_levels(0, argv);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::err);
    spdlog::set_level(spdlog::level::info);
}
