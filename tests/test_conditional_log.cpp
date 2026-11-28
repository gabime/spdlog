/*
 * This content is released under the MIT License as specified in
 * https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */

#include "includes.h"
#include "utils.h"

#if SPDLOG_ACTIVE_LEVEL != SPDLOG_LEVEL_DEBUG
#error "Invalid SPDLOG_ACTIVE_LEVEL in test. Should be SPDLOG_LEVEL_DEBUG"
#endif

#define THROTTLE_FILENAME "test_logs/throttle_log"

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
    REQUIRE(file_contents("test_logs/every_n_log").find("Progress: 0" SPDLOG_EOL) == 0);
    spdlog::drop("every_n_logger");
}

TEST_CASE("per-level every_n macros", "[conditional]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T("test_logs/every_n_level_log");

    auto logger =
        spdlog::create<spdlog::sinks::basic_file_sink_mt>("every_n_level_logger", filename);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);

    for (int i = 0; i < 50; ++i) {
        SPDLOG_LOGGER_INFO_EVERY_N(logger, 10, "info every {}", i);
        SPDLOG_LOGGER_WARN_EVERY_N(logger, 10, "warn every {}", i);
    }
    logger->flush();

    REQUIRE(count_lines("test_logs/every_n_level_log") == 10);
    spdlog::drop("every_n_level_logger");
}

TEST_CASE("throttle log", "[conditional]") {
    prepare_logdir();

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>(
        "throttle_logger", SPDLOG_FILENAME_T(THROTTLE_FILENAME));

    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);

    // 1. long interval: only the first call in a tight loop logs
    for (int i = 0; i < 100; ++i) {
        SPDLOG_LOGGER_THROTTLE(logger, spdlog::level::info, std::chrono::seconds(10), "event {}",
                               i);
    }
    logger->flush();
    REQUIRE(count_lines(THROTTLE_FILENAME) == 1);
    REQUIRE(file_contents(THROTTLE_FILENAME).find("event 0" SPDLOG_EOL) == 0);

    // 2. short interval: logs again after the interval has passed
    for (int round = 0; round < 2; ++round) {
        SPDLOG_LOGGER_THROTTLE(logger, spdlog::level::info, std::chrono::milliseconds(100),
                               "tick {}", round);
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
    logger->flush();
    REQUIRE(count_lines(THROTTLE_FILENAME) == 3);

    // 3. many threads, one call site, long interval -> exactly one line
    std::vector<std::thread> threads;
    for (int t = 0; t < 8; ++t) {
        threads.emplace_back([&logger]() {
            for (int i = 0; i < 1000; ++i) {
                SPDLOG_LOGGER_THROTTLE(logger, spdlog::level::info, std::chrono::seconds(10),
                                       "mt {}", i);
            }
        });
    }
    for (auto& th : threads) th.join();
    logger->flush();
    REQUIRE(count_lines(THROTTLE_FILENAME) == 4);

    // 4. zero interval logs every call
    for (int i = 0; i < 5; ++i) {
        SPDLOG_LOGGER_THROTTLE(logger, spdlog::level::info, std::chrono::seconds(0), "always {}",
                               i);
    }
    logger->flush();
    REQUIRE(count_lines(THROTTLE_FILENAME) == 9);

    spdlog::drop("throttle_logger");
}

TEST_CASE("per-level throttle macros", "[conditional]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T("test_logs/throttle_level_log");

    auto logger =
        spdlog::create<spdlog::sinks::basic_file_sink_mt>("throttle_level_logger", filename);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);

    // Zero interval -> every call fires
    SPDLOG_LOGGER_INFO_THROTTLE(logger, std::chrono::seconds(0), "info throttle {}", 1);
    SPDLOG_LOGGER_WARN_THROTTLE(logger, std::chrono::seconds(0), "warn throttle {}", 2);
    logger->flush();
    REQUIRE(count_lines("test_logs/throttle_level_log") == 2);

    spdlog::drop("throttle_level_logger");
}

TEST_CASE("every_n_state non-positive n", "[conditional]") {
    spdlog::details::every_n_state zero, one, negative;
    for (int i = 0; i < 5; ++i) {
        REQUIRE(zero.should_log(0));
        REQUIRE(one.should_log(1));
        REQUIRE(negative.should_log(-3));
    }
}

TEST_CASE("throttle_state edge intervals", "[conditional]") {
    spdlog::details::throttle_state zero, negative, huge, at_max;
    int zero_fired = 0, negative_fired = 0, huge_fired = 0, max_fired = 0;
    for (int i = 0; i < 5; ++i) {
        zero_fired += zero.should_log(std::chrono::milliseconds(0));
        negative_fired += negative.should_log(std::chrono::milliseconds(-1));
        // beyond the steady_clock range (~292 years): saturates and fires once
        huge_fired += huge.should_log(std::chrono::hours(24 * 365 * 300));
        max_fired += at_max.should_log((std::chrono::milliseconds::max)());
    }
    REQUIRE(zero_fired == 5);
    REQUIRE(negative_fired == 5);
    REQUIRE(huge_fired == 1);
    REQUIRE(max_fired == 1);
}

TEST_CASE("throttle_state zero interval is lossless under concurrency", "[conditional]") {
    spdlog::details::throttle_state state;
    std::atomic<int> fired{0};
    std::vector<std::thread> threads;
    for (int t = 0; t < 8; ++t) {
        threads.emplace_back([&state, &fired]() {
            for (int i = 0; i < 10000; ++i) {
                if (state.should_log(std::chrono::milliseconds(0))) {
                    ++fired;
                }
            }
        });
    }
    for (auto& th : threads) {
        th.join();
    }
    REQUIRE(fired.load() == 80000);
}
