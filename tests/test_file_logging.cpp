/*
 * This content is released under the MIT License as specified in
 * https://raw.githubusercontent.com/gabime/spdlog/v2.x/LICENSE
 */
#include "includes.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"

#define SIMPLE_LOG "test_logs/simple_log"
#define ROTATING_LOG "test_logs/rotating_log"

using namespace spdlog::sinks;

TEST_CASE("simple_file_logger", "[simple_logger]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T(SIMPLE_LOG);
    auto logger = spdlog::create<basic_file_sink_mt>("test-error", filename);
    logger->set_pattern("%v");
    logger->info("Test message {}", 1);
    logger->info("Test message {}", 2);
    logger->flush();
    require_message_count(SIMPLE_LOG, 2);
    using spdlog::details::os::default_eol;
    REQUIRE(file_contents(SIMPLE_LOG) == spdlog::fmt_lib::format("Test message 1{}Test message 2{}", default_eol, default_eol));
}

TEST_CASE("flush_on", "[flush_on]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T(SIMPLE_LOG);
    auto logger = spdlog::create<basic_file_sink_mt>("test-error", filename);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::info);
    logger->trace("Should not be flushed");
    REQUIRE(count_lines(SIMPLE_LOG) == 0);
    logger->info("Test message {}", 1);
    logger->info("Test message {}", 2);
    require_message_count(SIMPLE_LOG, 3);
    using spdlog::details::os::default_eol;
    REQUIRE(file_contents(SIMPLE_LOG) == spdlog::fmt_lib::format("Should not be flushed{}Test message 1{}Test message 2{}",
                                                                 default_eol, default_eol, default_eol));
}

TEST_CASE("basic_file_sink_truncate", "[truncate]") {
    prepare_logdir();
    const spdlog::filename_t filename = SPDLOG_FILENAME_T(SIMPLE_LOG);
    const bool truncate = true;
    const auto sink = std::make_shared<basic_file_sink_mt>(filename, truncate);
    const auto logger = std::make_shared<spdlog::logger>("simple_file_logger", sink);

    logger->info("Test message {}", 3.14);
    logger->info("Test message {}", 2.71);
    logger->flush();
    REQUIRE(count_lines(SIMPLE_LOG) == 2);

    sink->truncate();
    REQUIRE(count_lines(SIMPLE_LOG) == 0);

    logger->info("Test message {}", 6.28);
    logger->flush();
    REQUIRE(count_lines(SIMPLE_LOG) == 1);
}

TEST_CASE("rotating_file_logger1", "[rotating_logger]") {
    prepare_logdir();
    size_t max_size = 1024 * 10;
    spdlog::filename_t basename = SPDLOG_FILENAME_T(ROTATING_LOG);
    auto logger = spdlog::create<rotating_file_sink_mt>("logger", basename, max_size, 0);
    for (int i = 0; i < 10; ++i) {
        logger->info("Test message {}", i);
    }

    logger->flush();
    require_message_count(ROTATING_LOG, 10);
}

TEST_CASE("rotating_file_logger2", "[rotating_logger]") {
    prepare_logdir();
    size_t max_size = 1024 * 10;
    spdlog::filename_t basename = SPDLOG_FILENAME_T(ROTATING_LOG);
    {
        // make an initial logger to create the first output file
        auto logger = spdlog::create<rotating_file_sink_mt>("logger", basename, max_size, 2, true);
        for (int i = 0; i < 10; ++i) {
            logger->info("Test message {}", i);
        }
    }
    auto logger = spdlog::create<rotating_file_sink_mt>("logger", basename, max_size, 2, true);
    for (int i = 0; i < 10; ++i) {
        logger->info("Test message {}", i);
    }

    logger->flush();

    require_message_count(ROTATING_LOG, 10);

    for (int i = 0; i < 1000; i++) {
        logger->info("Test message {}", i);
    }

    logger->flush();
    REQUIRE(get_filesize(ROTATING_LOG) <= max_size);
    REQUIRE(get_filesize(ROTATING_LOG ".1") <= max_size);
}

// test that passing max_size=0 throws
TEST_CASE("rotating_file_logger3", "[rotating_logger]") {
    prepare_logdir();
    size_t max_size = 0;
    spdlog::filename_t basename = SPDLOG_FILENAME_T(ROTATING_LOG);
    REQUIRE_THROWS_AS(spdlog::create<rotating_file_sink_mt>("logger", basename, max_size, 0), spdlog::spdlog_ex);
}

// test on-demand rotation of logs
TEST_CASE("rotating_file_logger4", "[rotating_logger]") {
    prepare_logdir();
    size_t max_size = 1024 * 10;
    spdlog::filename_t basename = SPDLOG_FILENAME_T(ROTATING_LOG);
    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(basename, max_size, 2);
    auto logger = std::make_shared<spdlog::logger>("rotating_sink_logger", sink);
    logger->info("Test message - pre-rotation");
    logger->flush();
    sink->rotate_now();
    logger->info("Test message - post-rotation");
    logger->flush();
    REQUIRE(get_filesize(ROTATING_LOG) > 0);
    REQUIRE(get_filesize(ROTATING_LOG ".1") > 0);
}
