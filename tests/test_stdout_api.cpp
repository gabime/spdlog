/*
 * This content is released under the MIT License as specified in
 * https://raw.githubusercontent.com/gabime/spdlog/v2.x/LICENSE
 */
#include "includes.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/stdout_sinks.h"

TEST_CASE("stdout_st", "[stdout]") {
    spdlog::set_pattern("%+");
    auto l = spdlog::stdout_logger_st("test");
    l->set_level(spdlog::level::trace);
    l->trace("Test stdout_st");
    l->debug("Test stdout_st");
    l->info("Test stdout_st");
    l->warn("Test stdout_st");
    l->error("Test stdout_st");
    l->critical("Test stdout_st");
    spdlog::drop_all();
}

TEST_CASE("stderr_st", "[stderr]") {
    auto l = spdlog::stderr_logger_st("test");
    l->set_level(spdlog::level::trace);
    l->trace("Test stderr_st");
    l->debug("Test stderr_st");
    l->info("Test stderr_st");
    l->warn("Test stderr_st");
    l->error("Test stderr_st");
    l->critical("Test stderr_st");
    spdlog::drop_all();
}

TEST_CASE("stdout_mt", "[stdout]") {
    auto sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
    spdlog::logger logger("logger", sink);
    logger.debug("Test stdout_mt should not be displayed !!!!");
}

TEST_CASE("stderr_mt", "[stderr]") {
    auto sink = std::make_shared<spdlog::sinks::stderr_sink_mt>();
    spdlog::logger logger("logger", sink);
    logger.debug("Test stderr_mt should not be displayed !!!!");
}

// color loggers
TEST_CASE("stdout_color_st", "[stdout]") {
    auto l = spdlog::stdout_color_st("test");
    l->set_pattern("%+");
    l->set_level(spdlog::level::trace);
    l->trace("Test stdout_color_st");
    l->debug("Test stdout_color_st");
    l->info("Test stdout_color_st");
    l->warn("Test stdout_color_st");
    l->error("Test stdout_color_st");
    l->critical("Test stdout_color_st");
    spdlog::drop_all();
}

TEST_CASE("stdout_color_mt", "[stdout]") {
    auto l = spdlog::stdout_color_mt("test");
    l->set_pattern("%+");
    l->set_level(spdlog::level::trace);
    l->trace("Test stdout_color_mt");
    l->debug("Test stdout_color_mt");
    l->info("Test stdout_color_mt");
    l->warn("Test stdout_color_mt");
    l->error("Test stdout_color_mt");
    l->critical("Test stdout_color_mt");
    spdlog::drop_all();
}

TEST_CASE("stderr_color_st", "[stderr]") {
    auto l = spdlog::stderr_color_st("test");
    l->set_pattern("%+");
    l->set_level(spdlog::level::debug);
    l->debug("Test stderr_color_st");
    spdlog::drop_all();
}

TEST_CASE("stderr_color_mt", "[stderr]") {
    auto l = spdlog::stderr_color_mt("test");
    l->set_pattern("%+");
    l->info("Test stderr_color_mt");
    l->warn("Test stderr_color_mt");
    l->error("Test stderr_color_mt");
    l->critical("Test stderr_color_mt");
    spdlog::drop_all();
}
