/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"


TEST_CASE("simple_file_logger", "[simple_logger]]")
{
    prepare_logdir();
    std::string filename = "logs/simple_log";

    auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("logger", filename);
    logger->set_pattern("%v");

#if !defined(SPDLOG_FMT_PRINTF)
    logger->info("Test message {}", 1);
    logger->info("Test message {}", 2);
#else
    logger->info("Test message %d", 1);
    logger->info("Test message %d", 2);
#endif
    logger->flush();
    REQUIRE(file_contents(filename) == std::string("Test message 1\nTest message 2\n"));
    REQUIRE(count_lines(filename) == 2);
}


TEST_CASE("flush_on", "[flush_on]]")
{
    prepare_logdir();
    std::string filename = "logs/simple_log";

    auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("logger", filename);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::info);
    logger->trace("Should not be flushed");
    REQUIRE(count_lines(filename) == 0);

#if !defined(SPDLOG_FMT_PRINTF)
    logger->info("Test message {}", 1);
    logger->info("Test message {}", 2);
#else
    logger->info("Test message %d", 1);
    logger->info("Test message %d", 2);
#endif
    logger->flush();
    REQUIRE(file_contents(filename) == std::string("Should not be flushed\nTest message 1\nTest message 2\n"));
    REQUIRE(count_lines(filename) == 3);
}

TEST_CASE("rotating_file_logger1", "[rotating_logger]]")
{
    prepare_logdir();
    std::string basename = "logs/rotating_log";
    auto logger = spdlog::rotating_logger_mt("logger", basename, 1024, 0);

    for (int i = 0; i < 10; ++i)
    {
#if !defined(SPDLOG_FMT_PRINTF)
        logger->info("Test message {}", i);
#else
        logger->info("Test message %d", i);
#endif
    }

    logger->flush();
    auto filename = basename;
    REQUIRE(count_lines(filename) == 10);
}


TEST_CASE("rotating_file_logger2", "[rotating_logger]]")
{
    prepare_logdir();
    std::string basename = "logs/rotating_log";
    auto logger = spdlog::rotating_logger_mt("logger", basename, 1024, 1);
    for (int i = 0; i < 10; ++i)
        logger->info("Test message {}", i);

    logger->flush();
    auto filename = basename;
    REQUIRE(count_lines(filename) == 10);
    for (int i = 0; i < 1000; i++)
    {
#if !defined(SPDLOG_FMT_PRINTF)
        logger->info("Test message {}", i);
#else
        logger->info("Test message %d", i);
#endif
    }

    logger->flush();
    REQUIRE(get_filesize(filename) <= 1024);
    auto filename1 = basename + ".1";
    REQUIRE(get_filesize(filename1) <= 1024);
}


TEST_CASE("daily_logger", "[daily_logger]]")
{
    prepare_logdir();
    //calculate filename (time based)
    std::string basename = "logs/daily_log";
    std::tm tm = spdlog::details::os::localtime();
    fmt::MemoryWriter w;
    w.write("{}_{:04d}-{:02d}-{:02d}_{:02d}-{:02d}", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);

    auto logger = spdlog::daily_logger_mt("logger", basename, 0, 0);
    logger->flush_on(spdlog::level::info);
    for (int i = 0; i < 10; ++i)
    {
#if !defined(SPDLOG_FMT_PRINTF)
        logger->info("Test message {}", i);
#else
        logger->info("Test message %d", i);
#endif
    }

    auto filename = w.str();
    REQUIRE(count_lines(filename) == 10);
}


TEST_CASE("daily_logger with dateonly calculator", "[daily_logger_dateonly]]")
{
    using sink_type = spdlog::sinks::daily_file_sink<
                      std::mutex,
                      spdlog::sinks::dateonly_daily_file_name_calculator>;

    prepare_logdir();
    //calculate filename (time based)
    std::string basename = "logs/daily_dateonly";
    std::tm tm = spdlog::details::os::localtime();
    fmt::MemoryWriter w;
    w.write("{}_{:04d}-{:02d}-{:02d}", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    auto logger = spdlog::create<sink_type>("logger", basename, 0, 0);
    for (int i = 0; i < 10; ++i)
    {
#if !defined(SPDLOG_FMT_PRINTF)
        logger->info("Test message {}", i);
#else
        logger->info("Test message %d", i);
#endif
    }
    logger->flush();
    auto filename = w.str();
    REQUIRE(count_lines(filename) == 10);
}

struct custom_daily_file_name_calculator
{
    static spdlog::filename_t calc_filename(const spdlog::filename_t& basename)
    {
        std::tm tm = spdlog::details::os::localtime();
        fmt::MemoryWriter w;
        w.write("{}{:04d}{:02d}{:02d}", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
        return w.str();
    }
};

TEST_CASE("daily_logger with custom calculator", "[daily_logger_custom]]")
{
    using sink_type = spdlog::sinks::daily_file_sink<
                      std::mutex,
                      custom_daily_file_name_calculator>;

    prepare_logdir();
    //calculate filename (time based)
    std::string basename = "logs/daily_dateonly";
    std::tm tm = spdlog::details::os::localtime();
    fmt::MemoryWriter w;
    w.write("{}{:04d}{:02d}{:02d}", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    auto logger = spdlog::create<sink_type>("logger", basename, 0, 0);
    for (int i = 0; i < 10; ++i)
    {
#if !defined(SPDLOG_FMT_PRINTF)
        logger->info("Test message {}", i);
#else
        logger->info("Test message %d", i);
#endif
    }

    logger->flush();
    auto filename = w.str();
    REQUIRE(count_lines(filename) == 10);
}

