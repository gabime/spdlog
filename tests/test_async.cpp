#include "includes.h"
#include "test_sink.h"

template<class T>
std::string log_info(const T &what, spdlog::level::level_enum logger_level = spdlog::level::info)
{

    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

    spdlog::logger oss_logger("oss", oss_sink);
    oss_logger.set_level(logger_level);
    oss_logger.set_pattern("%v");
    oss_logger.info(what);

    return oss.str().substr(0, oss.str().length() - strlen(spdlog::details::os::default_eol));
}

TEST_CASE("basic async test ", "[async]")
{
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_mt>();
    size_t queue_size = 128;
    size_t messages = 256;
    auto logger = spdlog::create_async("as", test_sink, queue_size, spdlog::async_overflow_policy::block_retry);
    for (size_t i = 0; i < messages; i++)
    {
        logger->info("Hello message #{}", i);
    }

    // the dtor wait for all messages in the queue to get processed
    logger.reset();
    spdlog::drop("as");
    REQUIRE(test_sink->msg_counter() == messages);
    REQUIRE(test_sink->flushed_msg_counter() == messages);
}

TEST_CASE("discard policy ", "[async]")
{
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_mt>();
    size_t queue_size = 2;
    size_t messages = 1024;
    spdlog::drop("as");
    auto logger = spdlog::create_async("as", test_sink, queue_size, spdlog::async_overflow_policy::discard_log_msg);
    for (size_t i = 0; i < messages; i++)
    {
        logger->info("Hello message #{}", i);
    }

    // the dtor wait for all messages in the queue to get processed
    logger.reset();
    spdlog::drop("as");
    REQUIRE(test_sink->msg_counter() < messages);
    REQUIRE(test_sink->flushed_msg_counter() < messages);
}

TEST_CASE("flush", "[async]")
{
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_mt>();
    size_t queue_size = 256;
    size_t messages = 256;
    spdlog::drop("as");
    auto logger = std::make_shared<spdlog::async_logger>("as", test_sink, queue_size);
    for (size_t i = 0; i < messages; i++)
    {
        logger->info("Hello message #{}", i);
    }

    // the dtor wait for all messages in the queue to get processed
    logger->flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    REQUIRE(test_sink->msg_counter() == messages);
    REQUIRE(test_sink->flushed_msg_counter() == messages);

    REQUIRE(test_sink->flushed_msg_counter() == messages);
}

TEST_CASE("multi threads", "[async]")
{
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_mt>();
    size_t queue_size = 128;
    size_t messages = 256;
    size_t n_threads = 10;
    auto logger = std::make_shared<spdlog::async_logger>("as", test_sink, queue_size);

    std::vector<std::thread> threads;
    for (size_t i = 0; i < n_threads; i++)
    {
        threads.emplace_back([logger, messages] {
            for (size_t j = 0; j < messages; j++)
            {
                logger->info("Hello message #{}", j);
            }
        });
    }

    for (auto &t : threads)
    {
        t.join();
    }

    // the dtor wait for all messages in the queue to get processed
    logger.reset();
    REQUIRE(test_sink->msg_counter() == messages * n_threads);
    REQUIRE(test_sink->flushed_msg_counter() == messages * n_threads);
}

TEST_CASE("to_file", "[async]")
{

    prepare_logdir();
    size_t queue_size = 512;
    size_t messages = 512;
    size_t n_threads = 4;
    auto file_sink = std::make_shared<spdlog::sinks::simple_file_sink_mt>("logs/async_test.log", true);
    auto logger = spdlog::create_async("as", file_sink, queue_size);
    std::vector<std::thread> threads;
    for (size_t i = 0; i < n_threads; i++)
    {
        threads.emplace_back([logger, messages] {
            for (size_t j = 0; j < messages; j++)
            {
                logger->info("Hello message #{}", j);
            }
        });
    }

    for (auto &t : threads)
    {
        t.join();
    }
    logger.reset();
    spdlog::drop("as");
    REQUIRE(count_lines("logs/async_test.log") == messages * n_threads);
}
