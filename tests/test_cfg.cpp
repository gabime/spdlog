
#include "includes.h"
#include "test_sink.h"

#include <spdlog/cfg/env.h>
#include <spdlog/cfg/argv.h>

using spdlog::cfg::load_argv_levels;
using spdlog::cfg::load_env_levels;
using spdlog::sinks::test_sink_st;

TEST_CASE("env", "[cfg]")
{
#ifdef CATCH_PLATFORM_WINDOWS
    _putenv_s("SPDLOG_LEVEL", "warn");
#else
    setenv("SPDLOG_LEVEL", "warn", 1);
#endif
    auto levels = load_env_levels();
    REQUIRE(levels == spdlog::cfg::level_map{{"", spdlog::level::warn}});
}

TEST_CASE("argv1", "[cfg]")
{
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=l1=warn"};
    auto levels = load_argv_levels((int)(sizeof(argv) / sizeof(const char *)), argv);
    REQUIRE(levels == spdlog::cfg::level_map{{"l1", spdlog::level::warn}});
}

TEST_CASE("argv2", "[cfg]")
{
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=l1=warn,trace"};
    auto levels = load_argv_levels((int)(sizeof(argv) / sizeof(const char *)), argv);
    REQUIRE(levels == spdlog::cfg::level_map{{"l1", spdlog::level::warn}, {"", spdlog::level::trace}});
}

TEST_CASE("argv3", "[cfg]")
{
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=junk_name=warn"};
    auto levels = load_argv_levels((int)(sizeof(argv) / sizeof(const char *)), argv);
    REQUIRE(levels == spdlog::cfg::level_map{{"junk_name", spdlog::level::warn}});
}

TEST_CASE("argv4", "[cfg]")
{
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=junk"};
    auto levels = load_argv_levels((int)(sizeof(argv) / sizeof(const char *)), argv);
    REQUIRE(levels.empty());
}

TEST_CASE("argv5", "[cfg]")
{
    const char *argv[] = {"ignore", "ignore", "SPDLOG_LEVEL=l1=warn,trace"};
    auto levels = load_argv_levels((int)(sizeof(argv) / sizeof(const char *)), argv);
    REQUIRE(levels == spdlog::cfg::level_map{{"l1", spdlog::level::warn}, {"", spdlog::level::trace}});
}

TEST_CASE("argv6", "[cfg]")
{
    const char *argv[] = {"SPDLOG_LEVEL=info"};
    auto levels = load_argv_levels((int)(sizeof(argv) / sizeof(const char *)), argv);
    REQUIRE(levels.empty());
}

TEST_CASE("argv7", "[cfg]")
{
    const char *argv[] = {"ignore", ""};
    auto levels = load_argv_levels((int)(sizeof(argv) / sizeof(const char *)), argv);
    REQUIRE(levels.empty());
}

TEST_CASE("argv8", "[cfg]")
{
    const char *argv[] = {"ignore"};
    auto levels = load_argv_levels((int)(sizeof(argv) / sizeof(const char *)), argv);
    REQUIRE(levels.empty());
}

TEST_CASE("argv9", "[cfg]")
{
    const char *argv[] = {"ignore", "arg1"};
    auto levels = load_argv_levels((int)(sizeof(argv) / sizeof(const char *)), argv);
    REQUIRE(levels.empty());
}
TEST_CASE("argv10", "[cfg]")
{
    const char *argv[] = {"ignore", "junk", "SPDLOG_LEVEL=info"};
    auto levels = load_argv_levels((int)(sizeof(argv) / sizeof(const char *)), argv);
    REQUIRE(levels == spdlog::cfg::level_map{{"", spdlog::level::info}});
}

TEST_CASE("argv11", "[cfg]")
{
    const char *argv[] = {"ignore", "junk", "SPDLOG_LEVEL=info"};
    auto levels = load_argv_levels(0, argv);
    REQUIRE(levels.empty());
}

TEST_CASE("argv12", "[cfg]")
{
    const char *argv[] = {"ignore", "junk", "SPDLOG_LEVEL=info,l1=debug,l2=warn,l3=off"};
    auto levels = load_argv_levels((int)(sizeof(argv) / sizeof(const char *)), argv);
    REQUIRE(levels == spdlog::cfg::level_map{{"", spdlog::level::info}, {"l1", spdlog::level::debug}, {"l2", spdlog::level::warn},
                          {"l3", spdlog::level::off}});
}

TEST_CASE("argv13", "[cfg]")
{
    // test that only first  SPDLOG_LEVEL is handled
    const char *argv[] = {"ignore", "junk", "SPDLOG_LEVEL=info,l1=debug,l2=warn,l3=off", "SPDLOG_LEVEL=off"};
    auto levels = load_argv_levels((int)(sizeof(argv) / sizeof(const char *)), argv);
    REQUIRE(levels == spdlog::cfg::level_map{{"", spdlog::level::info}, {"l1", spdlog::level::debug}, {"l2", spdlog::level::warn},
                          {"l3", spdlog::level::off}});
}

TEST_CASE("argv14", "[cfg]")
{
    // test that only first  SPDLOG_LEVEL is handled
    const char *argv[] = {"ignore", "SPDLOG_LEVEL="};
    auto levels = load_argv_levels((int)(sizeof(argv) / sizeof(const char *)), argv);
    REQUIRE(levels.empty());
}

TEST_CASE("argv15", "[cfg]")
{
    // test that only first  SPDLOG_LEVEL is handled
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=info,l1=junk,l2=debug"};
    auto levels = load_argv_levels((int)(sizeof(argv) / sizeof(const char *)), argv);
    REQUIRE(levels == spdlog::cfg::level_map{
                          {"", spdlog::level::info},
                          {"l2", spdlog::level::debug},
                      });
}
