#include "includes.h"

static const char *logger_name = "null_logger";

TEST_CASE("register_drop", "[registry]")
{
    spdlog::drop_all();
    spdlog::create<spdlog::sinks::null_sink_mt>(logger_name);
    REQUIRE(spdlog::get(logger_name)!=nullptr);
    //Throw if registring existing name
    REQUIRE_THROWS_AS(spdlog::create<spdlog::sinks::null_sink_mt>(logger_name), spdlog::spdlog_ex);
}


TEST_CASE("explicit register" "[registry]")
{
    spdlog::drop_all();
    auto logger = std::make_shared<spdlog::logger>(logger_name, std::make_shared<spdlog::sinks::null_sink_st>());
    spdlog::register_logger(logger);
    REQUIRE(spdlog::get(logger_name) != nullptr);
    //Throw if registring existing name
    REQUIRE_THROWS_AS(spdlog::create<spdlog::sinks::null_sink_mt>(logger_name), spdlog::spdlog_ex);
}

TEST_CASE("drop" "[registry]")
{
    spdlog::drop_all();
    spdlog::create<spdlog::sinks::null_sink_mt>(logger_name);
    spdlog::drop(logger_name);
    REQUIRE_FALSE(spdlog::get(logger_name));
}

TEST_CASE("drop_all" "[registry]")
{
    spdlog::drop_all();
    spdlog::create<spdlog::sinks::null_sink_mt>(logger_name);
    spdlog::create<spdlog::sinks::null_sink_mt>("name2");
    spdlog::drop_all();
    REQUIRE_FALSE(spdlog::get(logger_name));
    REQUIRE_FALSE(spdlog::get("name2"));
}


TEST_CASE("drop non existing" "[registry]")
{
    spdlog::drop_all();
    spdlog::create<spdlog::sinks::null_sink_mt>(logger_name);
    spdlog::drop("some_name");
    REQUIRE_FALSE(spdlog::get("some_name"));
    REQUIRE(spdlog::get(logger_name));
    spdlog::drop_all();
}

