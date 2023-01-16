/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"
#include "test_sink.h"
#include "spdlog/sinks/callback_sink.h"
#include "spdlog/async.h"


TEST_CASE("custom_callback_logger", "[custom_callback_logger]]")
{
    spdlog::custom_log_callbacks callbacks;
    std::vector<std::string> lines;
    callbacks.on_log_formatted = [&](std::string str) { lines.push_back(str); };

    auto callback_logger = std::make_shared<spdlog::sinks::callback_sink_mt>(callbacks);
    std::shared_ptr<spdlog::sinks::test_sink_st> test_sink(new spdlog::sinks::test_sink_st);

    spdlog::logger logger("test-callback", {callback_logger, test_sink});

    logger.info("test message 1");
    logger.info("test message 2");
    logger.info("test message 3");

    std::vector<std::string> ref_lines = test_sink->lines();

    REQUIRE(lines[0] == ref_lines[0]);
    REQUIRE(lines[1] == ref_lines[1]);
    REQUIRE(lines[2] == ref_lines[2]);
    spdlog::drop_all();
}
