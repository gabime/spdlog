/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"

#define SIMPLE_LOG "test_logs/simple_log"
#define ROTATING_LOG "test_logs/rotating_log"

TEST_CASE("simple_file_logger", "[simple_logger]")
{
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T(SIMPLE_LOG);

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("logger", filename);
    logger->set_pattern("%v");

    logger->info("Test message {}", 1);
    logger->info("Test message {}", 2);

    logger->flush();
    require_message_count(SIMPLE_LOG, 2);
    using spdlog::details::os::default_eol;
    REQUIRE(file_contents(SIMPLE_LOG) == spdlog::fmt_lib::format("Test message 1{}Test message 2{}", default_eol, default_eol));
}

TEST_CASE("flush_on", "[flush_on]")
{
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T(SIMPLE_LOG);

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("logger", filename);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::info);
    logger->trace("Should not be flushed");
    REQUIRE(count_lines(SIMPLE_LOG) == 0);

    logger->info("Test message {}", 1);
    logger->info("Test message {}", 2);

    require_message_count(SIMPLE_LOG, 3);
    using spdlog::details::os::default_eol;
    REQUIRE(file_contents(SIMPLE_LOG) ==
            spdlog::fmt_lib::format("Should not be flushed{}Test message 1{}Test message 2{}", default_eol, default_eol, default_eol));
}

TEST_CASE("rotating_file_logger1", "[rotating_logger]")
{
    prepare_logdir();
    size_t max_size = 1024 * 10;
    spdlog::filename_t basename = SPDLOG_FILENAME_T(ROTATING_LOG);
    auto logger = spdlog::rotating_logger_mt("logger", basename, max_size, 0);

    for (int i = 0; i < 10; ++i)
    {
        logger->info("Test message {}", i);
    }

    logger->flush();
    require_message_count(ROTATING_LOG, 10);
}

TEST_CASE("rotating_file_logger2", "[rotating_logger]")
{
    prepare_logdir();
    size_t max_size = 1024 * 10;
    spdlog::filename_t basename = SPDLOG_FILENAME_T(ROTATING_LOG);

    {
        // make an initial logger to create the first output file
        auto logger = spdlog::rotating_logger_mt("logger", basename, max_size, 2, true);
        for (int i = 0; i < 10; ++i)
        {
            logger->info("Test message {}", i);
        }
        // drop causes the logger destructor to be called, which is required so the
        // next logger can rename the first output file.
        spdlog::drop(logger->name());
    }

    auto logger = spdlog::rotating_logger_mt("logger", basename, max_size, 2, true);
    for (int i = 0; i < 10; ++i)
    {
        logger->info("Test message {}", i);
    }

    logger->flush();

    require_message_count(ROTATING_LOG, 10);

    for (int i = 0; i < 1000; i++)
    {

        logger->info("Test message {}", i);
    }

    logger->flush();
    REQUIRE(get_filesize(ROTATING_LOG) <= max_size);
    REQUIRE(get_filesize(ROTATING_LOG ".1") <= max_size);
}

// test that passing max_size=0 throws
TEST_CASE("rotating_file_logger3", "[rotating_logger]")
{
    prepare_logdir();
    size_t max_size = 0;
    spdlog::filename_t basename = SPDLOG_FILENAME_T(ROTATING_LOG);
    REQUIRE_THROWS_AS(spdlog::rotating_logger_mt("logger", basename, max_size, 0), spdlog::spdlog_ex);
}
