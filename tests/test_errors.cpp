/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"

#include <iostream>

#define SIMPLE_LOG "test_logs/simple_log.txt"
#define SIMPLE_ASYNC_LOG "test_logs/simple_async_log.txt"

class failing_sink : public spdlog::sinks::base_sink<std::mutex>
{
protected:
    void sink_it_(const spdlog::details::log_msg &) final
    {
        throw std::runtime_error("some error happened during log");
    }

    void flush_() final
    {
        throw std::runtime_error("some error happened during flush");
    }
};

TEST_CASE("default_error_handler", "[errors]]")
{
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T(SIMPLE_LOG);

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("test-error", filename, true);
    logger->set_pattern("%v");
#ifdef SPDLOG_USE_STD_FORMAT
    logger->info("Test message {} {}", 1);
#else
    logger->info(fmt::runtime("Test message {} {}"), 1);
#endif
    logger->info("Test message {}", 2);
    logger->flush();

    using spdlog::details::os::default_eol;
    REQUIRE(file_contents(SIMPLE_LOG) == spdlog::fmt_lib::format("Test message 2{}", default_eol));
    REQUIRE(count_lines(SIMPLE_LOG) == 1);
}

struct custom_ex
{};
TEST_CASE("custom_error_handler", "[errors]]")
{
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T(SIMPLE_LOG);
    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("logger", filename, true);
    logger->flush_on(spdlog::level::info);
    logger->set_error_handler([=](const std::string &) { throw custom_ex(); });
    logger->info("Good message #1");

#ifdef SPDLOG_USE_STD_FORMAT
    REQUIRE_THROWS_AS(logger->info("Bad format msg {} {}", "xxx"), custom_ex);
#else
    REQUIRE_THROWS_AS(logger->info(fmt::runtime("Bad format msg {} {}"), "xxx"), custom_ex);
#endif
    logger->info("Good message #2");
    require_message_count(SIMPLE_LOG, 2);
}

TEST_CASE("default_error_handler2", "[errors]]")
{
    spdlog::drop_all();
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

    spdlog::filename_t filename = SPDLOG_FILENAME_T(SIMPLE_ASYNC_LOG);
    {
        spdlog::init_thread_pool(128, 1);
        auto logger = spdlog::create_async<spdlog::sinks::basic_file_sink_mt>("logger", filename, true);
        logger->set_error_handler([=](const std::string &) {
            std::ofstream ofs("test_logs/custom_err.txt");
            if (!ofs)
            {
                throw std::runtime_error("Failed open test_logs/custom_err.txt");
            }
            ofs << err_msg;
        });
        logger->info("Good message #1");
#ifdef SPDLOG_USE_STD_FORMAT
        logger->info("Bad format msg {} {}", "xxx");
#else
        logger->info(fmt::runtime("Bad format msg {} {}"), "xxx");
#endif
        logger->info("Good message #2");
        spdlog::drop("logger"); // force logger to drain the queue and shutdown
    }
    spdlog::init_thread_pool(128, 1);
    require_message_count(SIMPLE_ASYNC_LOG, 2);
    REQUIRE(file_contents("test_logs/custom_err.txt") == err_msg);
}

// Make sure async error handler is executed
TEST_CASE("async_error_handler2", "[errors]]")
{
    prepare_logdir();
    std::string err_msg("This is async handler error message");
    {
        spdlog::details::os::create_dir(SPDLOG_FILENAME_T("test_logs"));
        spdlog::init_thread_pool(128, 1);
        auto logger = spdlog::create_async<failing_sink>("failed_logger");
        logger->set_error_handler([=](const std::string &) {
            std::ofstream ofs("test_logs/custom_err2.txt");
            if (!ofs)
                throw std::runtime_error("Failed open test_logs/custom_err2.txt");
            ofs << err_msg;
        });
        logger->info("Hello failure");
        spdlog::drop("failed_logger"); // force logger to drain the queue and shutdown
    }

    spdlog::init_thread_pool(128, 1);
    REQUIRE(file_contents("test_logs/custom_err2.txt") == err_msg);
}
