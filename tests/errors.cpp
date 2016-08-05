/*
* This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
*/
#include "includes.h"

#include<iostream>


TEST_CASE("default_error_handler", "[errors]]")
{
    prepare_logdir();
    std::string filename = "logs/simple_log.txt";

    auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("logger", filename, true);
    logger->set_pattern("%v");
    logger->info("Test message {} {}", 1);
    logger->info("Test message {}", 2);
    logger->flush();

    REQUIRE(file_contents(filename) == std::string("Test message 2\n"));
    REQUIRE(count_lines(filename) == 1);
}


struct custom_ex {};
TEST_CASE("custom_error_handler", "[errors]]")
{
    prepare_logdir();
    std::string filename = "logs/simple_log.txt";
    auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("logger", filename, true);
    logger->set_error_handler([=](const std::string& msg)
    {
        throw custom_ex();
    });
    logger->info("Good message #1");
    REQUIRE_THROWS_AS(logger->info("Bad format msg {} {}", "xxx"), custom_ex);
    logger->info("Good message #2");
    REQUIRE(count_lines(filename) == 2);
}

TEST_CASE("async_error_handler", "[errors]]")
{
    prepare_logdir();
    std::string err_msg("log failed with some msg");
    spdlog::set_async_mode(128);
    std::string filename = "logs/simple_async_log.txt";
    {
        auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("logger", filename, true);
        logger->set_error_handler([=](const std::string& msg)
        {
            std::ofstream ofs("logs/custom_err.txt");
            if (!ofs) throw std::runtime_error("Failed open logs/custom_err.txt");
            ofs << err_msg;
        });
        logger->info("Good message #1");
        logger->info("Bad format msg {} {}", "xxx");
        logger->info("Good message #2");
        spdlog::drop("logger"); //force logger to drain the queue and shutdown
        spdlog::set_sync_mode();
    }
    REQUIRE(count_lines(filename) == 2);
    REQUIRE(file_contents("logs/custom_err.txt") == err_msg);
}
