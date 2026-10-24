/*
 * This content is released under the MIT License as specified in
 * https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */

#include "includes.h"
#include "utils.h"

#if SPDLOG_ACTIVE_LEVEL != SPDLOG_LEVEL_DEBUG
#error "Invalid SPDLOG_ACTIVE_LEVEL in test. Should be SPDLOG_LEVEL_DEBUG"
#endif

#define TEST_FILENAME "test_logs/simple_log"
#define THROTTLE_FILENAME "test_logs/throttle_log"

TEST_CASE("conditional log with formate string", "[conditional]") {
    prepare_logdir();

    spdlog::filename_t filename = SPDLOG_FILENAME_T(TEST_FILENAME);

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("cond_logger", filename);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);

    // test condition is true
    int error_count = 5;
    SPDLOG_LOGGER_CONDITIONAL(logger, error_count > 0, spdlog::level::err, "Found {} errors",
                              error_count);

    logger->flush();
    REQUIRE(count_lines(TEST_FILENAME) == 1);
    // REQUIRE(ends_with(file_contents(TEST_FILENAME), "Found 5 errors\n"));
    REQUIRE(ends_with(file_contents(TEST_FILENAME), "Found 5 errors" SPDLOG_EOL));
    // test condition is false
    error_count = 0;
    SPDLOG_LOGGER_CONDITIONAL(logger, error_count > 0, spdlog::level::err, "Found {} errors",
                              error_count);

    logger->flush();
    REQUIRE(count_lines(TEST_FILENAME) == 1);

    // arguments are not evaluated when the condition is false
    int calls = 0;
    SPDLOG_LOGGER_CONDITIONAL(logger, false, spdlog::level::err, "x {}", calls++);
    REQUIRE(calls == 0);

    spdlog::drop("cond_log");
}

TEST_CASE("every_n log", "[conditional]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T("test_logs/every_n_log");

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("every_n_logger", filename);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);

    for (int i = 0; i < 100; ++i) {
        SPDLOG_LOGGER_EVERY_N(logger, spdlog::level::info, 10, "Progress: {}", i);
    }
    logger->flush();

    REQUIRE(count_lines("test_logs/every_n_log") == 10);
    // REQUIRE(file_contents("test_logs/every_n_log").find("Progress: 0\n") == 0);
    REQUIRE(file_contents("test_logs/every_n_log").find("Progress: 0" SPDLOG_EOL) == 0);
    spdlog::drop("every_n_logger");
}

TEST_CASE("throttle log", "[conditional]") {
    prepare_logdir();
    // const char* path = "test_logs/throttle_log";

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>(
        "throttle_logger", SPDLOG_FILENAME_T(THROTTLE_FILENAME));

    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);

    // 1. long interval: only the first call in a tight loop logs
    for (int i = 0; i < 100; ++i) {
        SPDLOG_LOGGER_THROTTLE(logger, spdlog::level::info, 10.0, "event {}", i);
    }
    logger->flush();
    REQUIRE(count_lines(THROTTLE_FILENAME) == 1);
    // REQUIRE(file_contents(path).find("event 0\n") == 0);
    REQUIRE(file_contents(THROTTLE_FILENAME).find("event 0" SPDLOG_EOL) == 0);
    // 2. short interval: logs again after the interval has passed
    for (int round = 0; round < 2; ++round) {
        SPDLOG_LOGGER_THROTTLE(logger, spdlog::level::info, 0.1, "tick {}", round);
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
    logger->flush();
    REQUIRE(count_lines(THROTTLE_FILENAME) == 3);

    // 3. many threads, one call site, long interval -> exactly one line
    std::vector<std::thread> threads;
    for (int t = 0; t < 8; ++t) {
        threads.emplace_back([&logger]() {
            for (int i = 0; i < 1000; ++i) {
                SPDLOG_LOGGER_THROTTLE(logger, spdlog::level::info, 10.0, "mt {}", i);
            }
        });
    }
    for (auto& th : threads) th.join();
    logger->flush();
    REQUIRE(count_lines(THROTTLE_FILENAME) == 4);

    // 4. interval <= 0 logs every call
    for (int i = 0; i < 5; ++i) {
        SPDLOG_LOGGER_THROTTLE(logger, spdlog::level::info, 0.0, "always {}", i);
    }
    logger->flush();
    REQUIRE(count_lines(THROTTLE_FILENAME) == 9);

    spdlog::drop("throttle_logger");
}
