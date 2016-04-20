/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"


TEST_CASE("simple_file_logger", "[simple_logger]]")
{
    prepare_logdir();
    std::string filename = "logs/simple_log.txt";

    auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("logger", filename);
    logger->set_pattern("%v");


    logger->info("Test message {}", 1);
    logger->info("Test message {}", 2);
    logger->flush();
    REQUIRE(file_contents(filename) == std::string("Test message 1\nTest message 2\n"));
    REQUIRE(count_lines(filename) == 2);

}

TEST_CASE("rotating_file_logger1", "[rotating_logger]]")
{
    prepare_logdir();
    std::string basename = "logs/rotating_log";
    auto logger = spdlog::rotating_logger_mt("logger", basename, 1024, 0, true);
    for (int i = 0; i < 10; ++i)
        logger->info("Test message {}", i);

    auto filename = basename + ".txt";
    REQUIRE(count_lines(filename) == 10);
    for (int i = 0; i < 1000; i++)
        logger->info("Test message {}", i);

}


TEST_CASE("rotating_file_logger2", "[rotating_logger]]")
{
    prepare_logdir();
    std::string basename = "logs/rotating_log";
    auto logger = spdlog::rotating_logger_mt("logger", basename, 1024, 1, false);
    for (int i = 0; i < 10; ++i)
        logger->info("Test message {}", i);

    logger->flush();
    auto filename = basename + ".txt";
    REQUIRE(count_lines(filename) == 10);
    for (int i = 0; i < 1000; i++)
        logger->info("Test message {}", i);

    logger->flush();
    REQUIRE(get_filesize(filename) <= 1024);
    auto filename1 = basename + ".1.txt";
    REQUIRE(get_filesize(filename1) <= 1024);
}


TEST_CASE("daily_logger", "[daily_logger]]")
{

    prepare_logdir();
    //calculate filename (time based)
    std::string basename = "logs/daily_log";
    std::tm tm = spdlog::details::os::localtime();
    fmt::MemoryWriter w;
    w.write("{}_{:04d}-{:02d}-{:02d}_{:02d}-{:02d}.txt", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);

    auto logger = spdlog::daily_logger_mt("logger", basename, 0, 0, true);
    for (int i = 0; i < 10; ++i)
        logger->info("Test message {}", i);

    auto filename = w.str();
    REQUIRE(count_lines(filename) == 10);
}














