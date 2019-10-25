/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"

TEST_CASE("simple_file_logger", "[simple_logger]]")
{
    prepare_logdir();
    std::string filename = "test_logs/simple_log";

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("logger", filename);
    logger->set_pattern("%v");

    logger->info("Test message {}", 1);
    logger->info("Test message {}", 2);

    logger->flush();
    REQUIRE(file_contents(filename) == std::string("Test message 1\nTest message 2\n"));
    REQUIRE(count_lines(filename) == 2);
}

TEST_CASE("flush_on", "[flush_on]]")
{
    prepare_logdir();
    std::string filename = "test_logs/simple_log";

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("logger", filename);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::info);
    logger->trace("Should not be flushed");
    REQUIRE(count_lines(filename) == 0);

    logger->info("Test message {}", 1);
    logger->info("Test message {}", 2);

    REQUIRE(file_contents(filename) == std::string("Should not be flushed\nTest message 1\nTest message 2\n"));
    REQUIRE(count_lines(filename) == 3);
}

TEST_CASE("rotating_file_logger1", "[rotating_logger]]")
{
    prepare_logdir();
    size_t max_size = 1024 * 10;
    std::string basename = "test_logs/rotating_log";
    auto logger = spdlog::rotating_logger_mt("logger", basename, max_size, 0);

    for (int i = 0; i < 10; ++i)
    {
        logger->info("Test message {}", i);
    }

    logger->flush();
    auto filename = basename;
    REQUIRE(count_lines(filename) == 10);
}

TEST_CASE("rotating_file_logger2", "[rotating_logger]]")
{
    prepare_logdir();
    size_t max_size = 1024 * 10;
    std::string basename = "test_logs/rotating_log";

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
    auto filename = basename;
    REQUIRE(count_lines(filename) == 10);
    for (int i = 0; i < 1000; i++)
    {

        logger->info("Test message {}", i);
    }

    logger->flush();
    REQUIRE(get_filesize(filename) <= max_size);
    auto filename1 = basename + ".1";
    REQUIRE(get_filesize(filename1) <= max_size);
}
