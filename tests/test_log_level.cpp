#include <map>
#include <vector>

#include "includes.h"
#include "test_sink.h"

TEST_CASE("test_default_level", "[log_level]") {
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_st>();
    REQUIRE(test_sink->log_level() == spdlog::level::trace);
    spdlog::logger logger("test-level", test_sink);
    REQUIRE(test_sink->log_level() == spdlog::level::trace);
    REQUIRE(logger.log_level() == spdlog::level::info);
}

// test that logger log only messages with level bigger or equal to its level
void test_logger_level(spdlog::level level);
TEST_CASE("test_logger_levels", "[log_level]") {
    for (size_t i = 0; i < spdlog::levels_count; i++) {
        auto level = static_cast<spdlog::level>(i);
        test_logger_level(level);
    }
}

// test that logger log all messages with level bigger or equal to its level
void test_sink_level(spdlog::level level);
TEST_CASE("test_sink_levels", "[log_level]") {
    for (size_t i = 0; i < spdlog::levels_count; i++) {
        auto level = static_cast<spdlog::level>(i);
        test_sink_level(level);
    }
}

//
// test helpers to check that logger/sink displays only messages with level bigger or equal to its
// level
//
auto get_expected_messages(spdlog::level level) {
    // expected messages for each level
    static const std::map<spdlog::level, std::vector<std::string>> messages = {
        {spdlog::level::trace, {"trace hello", "debug hello", "info hello", "warning hello", "error hello", "critical hello"}},
        {spdlog::level::debug, {"debug hello", "info hello", "warning hello", "error hello", "critical hello"}},
        {spdlog::level::info, {"info hello", "warning hello", "error hello", "critical hello"}},
        {spdlog::level::warn, {"warning hello", "error hello", "critical hello"}},
        {spdlog::level::err, {"error hello", "critical hello"}},
        {spdlog::level::critical, {"critical hello"}},
        {spdlog::level::off, {}},
        {spdlog::level::n_levels, {}}};
    return messages.at(level);
}

void test_logger_level(spdlog::level level) {
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_st>();
    spdlog::logger logger("test-level", test_sink);
    logger.set_level(level);
    logger.set_pattern("%l %v");
    REQUIRE(logger.log_level() == level);
    REQUIRE(test_sink->log_level() == spdlog::level::trace);

    logger.trace("hello");
    logger.debug("hello");
    logger.info("hello");
    logger.warn("hello");
    logger.error("hello");
    logger.critical("hello");

    auto lines = test_sink->lines();
    REQUIRE(lines == get_expected_messages(level));
}

// test that sink displays all messages with level bigger or equal to its level
void test_sink_level(spdlog::level level) {
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_st>();
    spdlog::logger logger("test-level", test_sink);
    logger.set_level(spdlog::level::trace);
    test_sink->set_level(level);

    REQUIRE(logger.log_level() == spdlog::level::trace);
    REQUIRE(test_sink->log_level() == level);

    logger.set_pattern("%l %v");

    logger.trace("hello");
    logger.debug("hello");
    logger.info("hello");
    logger.warn("hello");
    logger.error("hello");
    logger.critical("hello");

    auto lines = test_sink->lines();
    REQUIRE(lines == get_expected_messages(level));
}