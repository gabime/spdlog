/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"

#include <iostream>

class failing_sink : public spdlog::sinks::sink
{
    void log(const spdlog::details::log_msg &) override
    {
        throw std::runtime_error("some error happened during log");
    }

    void flush() override
    {
        throw std::runtime_error("some error happened during flush");
    }
};
using namespace std;
TEST_CASE("default_error_handler", "[errors]]")
{
    prepare_logdir();
    std::string filename = "logs/simple_log.txt";

    auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("logger", filename, true);
    logger->set_pattern("%v");
#if !defined(SPDLOG_FMT_PRINTF)
    logger->info("Test message {} {}", 1);
    logger->info("Test message {}", 2);
#else
    logger->info("Test message %d %d", 1);
    logger->info("Test message %d", 2);
#endif
    logger->flush();

    REQUIRE(file_contents(filename) == std::string("Test message 2\n"));
    REQUIRE(count_lines(filename) == 1);
}

struct custom_ex
{
};
TEST_CASE("custom_error_handler", "[errors]]")
{
    prepare_logdir();
    std::string filename = "logs/simple_log.txt";
    auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("logger", filename, true);
    logger->flush_on(spdlog::level::info);
    logger->set_error_handler([=](const std::string &) { throw custom_ex(); });
    logger->info("Good message #1");
#if !defined(SPDLOG_FMT_PRINTF)
    REQUIRE_THROWS_AS(logger->info("Bad format msg {} {}", "xxx"), custom_ex);
#else
    REQUIRE_THROWS_AS(logger->info("Bad format msg %s %s", "xxx"), custom_ex);
#endif
    logger->info("Good message #2");
    REQUIRE(count_lines(filename) == 2);
}

TEST_CASE("default_error_handler2", "[errors]]")
{
    auto logger = spdlog::create<failing_sink>("failed_logger");
    logger->set_error_handler([=](const std::string &) { throw custom_ex(); });
    REQUIRE_THROWS_AS(logger->info("Some message"), custom_ex);
}

TEST_CASE("flush_error_handler", "[errors]]")
{
    spdlog::drop_all();
    auto logger = spdlog::create<failing_sink>("failed_logger");
    logger->set_error_handler([=](const std::string &) { throw custom_ex(); });
    REQUIRE_THROWS_AS(logger->flush(), custom_ex);
}

TEST_CASE("async_error_handler", "[errors]]")
{
    prepare_logdir();
    std::string err_msg("log failed with some msg");
    spdlog::set_async_mode(128);
    std::string filename = "logs/simple_async_log.txt";
    {
        auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("logger", filename, true);
        logger->set_error_handler([=](const std::string &) {
            std::ofstream ofs("logs/custom_err.txt");
            if (!ofs)
                throw std::runtime_error("Failed open logs/custom_err.txt");
            ofs << err_msg;
        });
        logger->info("Good message #1");
#if !defined(SPDLOG_FMT_PRINTF)
        logger->info("Bad format msg {} {}", "xxx");
#else
        logger->info("Bad format msg %s %s", "xxx");
#endif
        logger->info("Good message #2");
        spdlog::drop("logger"); // force logger to drain the queue and shutdown
        spdlog::set_sync_mode();
    }
    REQUIRE(count_lines(filename) == 2);
    REQUIRE(file_contents("logs/custom_err.txt") == err_msg);
}

// Make sure async error handler is executed
TEST_CASE("async_error_handler2", "[errors]]")
{
    prepare_logdir();
    std::string err_msg("This is async handler error message");
    spdlog::set_async_mode(128);
    {
        auto logger = spdlog::create<failing_sink>("failed_logger");
        logger->set_error_handler([=](const std::string &) {
            std::ofstream ofs("logs/custom_err2.txt");
            if (!ofs)
                throw std::runtime_error("Failed open logs/custom_err2.txt");
            ofs << err_msg;
        });
        logger->info("Hello failure");
        spdlog::drop("failed_logger"); // force logger to drain the queue and shutdown
        spdlog::set_sync_mode();
        logger.reset();
    }

    REQUIRE(file_contents("logs/custom_err2.txt") == err_msg);
}
