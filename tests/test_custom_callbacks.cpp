/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"
#include "test_sink.h"
#include "spdlog/sinks/callback_sink.h"
#include "spdlog/async.h"
#include "spdlog/common.h"

TEST_CASE("custom_callback_logger", "[custom_callback_logger]]")
{
    std::vector<std::string> lines;
    spdlog::pattern_formatter formatter;
    auto callback_logger = std::make_shared<spdlog::sinks::callback_sink_st>([&](const spdlog::details::log_msg &msg) {
        spdlog::memory_buf_t formatted;
        formatter.format(msg, formatted);
        auto eol_len = strlen(spdlog::details::os::default_eol);
        lines.emplace_back(formatted.begin(), formatted.end() - eol_len);
    });
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
