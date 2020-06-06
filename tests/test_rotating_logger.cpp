/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"

TEST_CASE("rotating_looger with default calculator", "[rotating_logger]")
{
    using sink_type = spdlog::sinks::rotating_file_sink<std::mutex, spdlog::sinks::rotating_filename_calculator>;

    prepare_logdir();

    std::string filename = "test_logs/rotating_default";
    
    auto logger = spdlog::create<sink_type>("logger", filename, 5 * 1024, 1);
    for (int i = 0; i < 10; ++i)
    {
        logger->info("Hello World!", i);
    }
    logger->flush();

    require_message_count(filename, 10);
}

struct custom_rotating_file_name_calculator
{
    static spdlog::filename_t calc_filename(const spdlog::filename_t &filename, std::size_t)
    {
        spdlog::filename_t basename, ext;
        std::tie(basename, ext) = spdlog::details::file_helper::split_by_extension(filename);
        return fmt::format(SPDLOG_FILENAME_T("{}_custom{}"), basename, ext);
    }
};

TEST_CASE("rotating_logger with custom calculator", "[rotating_logger]")
{
    using sink_type = spdlog::sinks::rotating_file_sink<std::mutex, custom_rotating_file_name_calculator>;

    prepare_logdir();

    // calculate filename
    std::string basename = "test_logs/rotating";
    spdlog::memory_buf_t w;
    fmt::format_to(w, "{}_custom", basename);

    auto logger = spdlog::create<sink_type>("logger", basename, 5 * 1024, 1);
    for (int i = 0; i < 10; ++i)
    {
        logger->info("Hello World!");
    }

    logger->flush();

    auto filename = fmt::to_string(w);
    require_message_count(filename, 10);
}

/*
 * File name calculations
 */

TEST_CASE("rotating_file_sink::calc_filename1", "[rotating_file_sink]]")
{
    auto filename = spdlog::sinks::rotating_filename_calculator::calc_filename("rotated.txt", 3);
    REQUIRE(filename == "rotated.3.txt");
}

TEST_CASE("rotating_file_sink::calc_filename2", "[rotating_file_sink]]")
{
    auto filename = spdlog::sinks::rotating_filename_calculator::calc_filename("rotated", 3);
    REQUIRE(filename == "rotated.3");
}

TEST_CASE("rotating_file_sink::calc_filename3", "[rotating_file_sink]]")
{
    auto filename = spdlog::sinks::rotating_filename_calculator::calc_filename("rotated.txt", 0);
    REQUIRE(filename == "rotated.txt");
}
