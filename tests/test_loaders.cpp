#include "includes.h"
#include "test_sink.h"

#include <spdlog/loaders/env.h>
#include <spdlog/loaders/argv.h>

using spdlog::loaders::load_argv;
using spdlog::loaders::load_env;

TEST_CASE("env", "[loaders]")
{
    spdlog::drop("l1");
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
#ifdef _MSVC
    _putenv_s("SPDLOG_LEVEL", "l1=warn");
#else
    setenv("SPDLOG_LEVEL", "l1=warn", 1);
#endif
    load_env();
    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
}

TEST_CASE("argv1", "[loaders]")
{
    spdlog::drop("l1");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=l1=warn"};
    load_argv(2, const_cast<char**>(argv));
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
}

TEST_CASE("argv2", "[loaders]")
{
    spdlog::drop("l1");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=l1=warn,trace"};
    load_argv(2, const_cast<char**>(argv));
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::trace);
}

TEST_CASE("argv3", "[loaders]")
{
    spdlog::drop("l1");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL="};
    load_argv(2, const_cast<char**>(argv));
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::info);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
}

TEST_CASE("argv4", "[loaders]")
{
    spdlog::drop("l1");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=junk"};
    load_argv(2, const_cast<char**>(argv));
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::info);
}

TEST_CASE("argv5", "[loaders]")
{
    spdlog::drop("l1");
    const char *argv[] = {"ignore", "ignore", "SPDLOG_LEVEL=l1=warn,trace"};
    load_argv(3, const_cast<char**>(argv));
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::trace);
}

TEST_CASE("argv6", "[loaders]")
{
    spdlog::set_level(spdlog::level::err);
    const char *argv[] = {""};
    load_argv(1, const_cast<char**>(argv));
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::err);

}

TEST_CASE("argv7", "[loaders]")
{
    spdlog::set_level(spdlog::level::err);
    const char *argv[] = {""};
    load_argv(0, const_cast<char**>(argv));
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::err);
}
