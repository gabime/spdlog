#include "includes.h"
#include "test_sink.h"

#include <spdlog/loaders/env.h>
#include <spdlog/loaders/argv.h>

using spdlog::loaders::load_argv;
using spdlog::loaders::load_env;

TEST_CASE("env", "[loaders]")
{
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    setenv("SPDLOG_LEVEL", "l1=warn", 1);
    load_env();
    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
    spdlog::drop(l1->name());
}

TEST_CASE("argv1", "[loaders]")
{
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=l1=warn"};
    load_argv(2, const_cast<char**>(argv));
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
    spdlog::drop(l1->name());
}

TEST_CASE("argv2", "[loaders]")
{
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=l1=warn,trace"};
    load_argv(2, const_cast<char**>(argv));
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::trace);
    spdlog::drop(l1->name());
}

TEST_CASE("argv3", "[loaders]")
{
    const char *argv[] = {"ignore", "SPDLOG_LEVEL="};
    load_argv(2, const_cast<char**>(argv));
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::info);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
    spdlog::drop(l1->name());
}

TEST_CASE("argv4", "[loaders]")
{
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=junk"};
    load_argv(2, const_cast<char**>(argv));
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::info);
    spdlog::drop(l1->name());
}
