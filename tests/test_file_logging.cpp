/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"

TEST_CASE("simple_file_logger", "[simple_logger]]")
{
    prepare_logdir();
    std::string filename = "logs/simple_log";

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
    std::string filename = "logs/simple_log";

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
    std::string basename = "logs/rotating_log";
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
    std::string basename = "logs/rotating_log";

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

TEST_CASE("daily_logger with dateonly calculator", "[daily_logger_dateonly]]")
{
    using sink_type = spdlog::sinks::daily_file_sink<std::mutex, spdlog::sinks::daily_filename_calculator>;

    prepare_logdir();
    // calculate filename (time based)
    std::string basename = "logs/daily_dateonly";
    std::tm tm = spdlog::details::os::localtime();
    fmt::memory_buffer w;
    fmt::format_to(w, "{}_{:04d}-{:02d}-{:02d}", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    auto logger = spdlog::create<sink_type>("logger", basename, 0, 0);
    for (int i = 0; i < 10; ++i)
    {

        logger->info("Test message {}", i);
    }
    logger->flush();
    auto filename = fmt::to_string(w);
    REQUIRE(count_lines(filename) == 10);
}

struct custom_daily_file_name_calculator
{
    static spdlog::filename_t calc_filename(const spdlog::filename_t &basename, const tm &now_tm)
    {
        fmt::memory_buffer w;
        fmt::format_to(w, "{}{:04d}{:02d}{:02d}", basename, now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);
        return fmt::to_string(w);
    }
};

TEST_CASE("daily_logger with custom calculator", "[daily_logger_custom]]")
{
    using sink_type = spdlog::sinks::daily_file_sink<std::mutex, custom_daily_file_name_calculator>;

    prepare_logdir();
    // calculate filename (time based)
    std::string basename = "logs/daily_dateonly";
    std::tm tm = spdlog::details::os::localtime();
    fmt::memory_buffer w;
    fmt::format_to(w, "{}{:04d}{:02d}{:02d}", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    auto logger = spdlog::create<sink_type>("logger", basename, 0, 0);
    for (int i = 0; i < 10; ++i)
    {
        logger->info("Test message {}", i);
    }

    logger->flush();
    auto filename = fmt::to_string(w);
    REQUIRE(count_lines(filename) == 10);
}

/*
 * File name calculations
 */

TEST_CASE("rotating_file_sink::calc_filename1", "[rotating_file_sink]]")
{
    auto filename = spdlog::sinks::rotating_file_sink_st::calc_filename("rotated.txt", 3);
    REQUIRE(filename == "rotated.3.txt");
}

TEST_CASE("rotating_file_sink::calc_filename2", "[rotating_file_sink]]")
{
    auto filename = spdlog::sinks::rotating_file_sink_st::calc_filename("rotated", 3);
    REQUIRE(filename == "rotated.3");
}

TEST_CASE("rotating_file_sink::calc_filename3", "[rotating_file_sink]]")
{
    auto filename = spdlog::sinks::rotating_file_sink_st::calc_filename("rotated.txt", 0);
    REQUIRE(filename == "rotated.txt");
}

// regex supported only from gcc 4.9 and above
#if defined(_MSC_VER) || !(__GNUC__ <= 4 && __GNUC_MINOR__ < 9)
#include <regex>

TEST_CASE("daily_file_sink::daily_filename_calculator", "[daily_file_sink]]")
{
    // daily_YYYY-MM-DD_hh-mm.txt
    auto filename = spdlog::sinks::daily_filename_calculator::calc_filename("daily.txt", spdlog::details::os::localtime());
    // date regex based on https://www.regular-expressions.info/dates.html
    std::regex re(R"(^daily_(19|20)\d\d-(0[1-9]|1[012])-(0[1-9]|[12][0-9]|3[01])\.txt$)");
    std::smatch match;
    REQUIRE(std::regex_match(filename, match, re));
}
#endif
