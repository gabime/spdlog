#include "includes.h"
#include "spdlog/async.h"
#include "spdlog/sinks/simple_file_sink.h"
#include "test_sink.h"

// std::unique_ptr<spdlog::async_logger> create_logger(size_t tp_queue_size, size_t tp_threads)
//{
//	auto tp = std::make_shared<details::thread_pool>(8192, 1);
//	auto logger = std::make_shared<async_logger>("as", test_sink, tp, async_overflow_policy::block_retry);
//}

TEST_CASE("basic async test ", "[async]")
{
    using namespace spdlog;
    auto test_sink = std::make_shared<sinks::test_sink_mt>();
    size_t queue_size = 128;
    size_t messages = 256;
    {
        auto tp = std::make_shared<details::thread_pool>(queue_size, 1);
        auto logger = std::make_shared<async_logger>("as", test_sink, tp, async_overflow_policy::block_retry);
        for (size_t i = 0; i < messages; i++)
        {
            logger->info("Hello message #{}", i);
        }
        logger->flush();
    }
    REQUIRE(test_sink->msg_counter() == messages);
    REQUIRE(test_sink->flushed_msg_counter() == messages);
}

TEST_CASE("discard policy ", "[async]")
{
    using namespace spdlog;
    auto test_sink = std::make_shared<sinks::test_sink_mt>();
    size_t queue_size = 2;
    size_t messages = 1024;
    {
        auto tp = std::make_shared<details::thread_pool>(queue_size, 1);
        auto logger = std::make_shared<async_logger>("as", test_sink, tp, async_overflow_policy::discard_log_msg);
        for (size_t i = 0; i < messages; i++)
        {
            logger->info("Hello message #{}", i);
        }
    }

    REQUIRE(test_sink->msg_counter() < messages);
    REQUIRE(test_sink->flushed_msg_counter() < messages);
}

TEST_CASE("flush", "[async]")
{
    using namespace spdlog;
    auto test_sink = std::make_shared<sinks::test_sink_mt>();
    size_t queue_size = 256;
    size_t messages = 256;
    {
        auto tp = std::make_shared<details::thread_pool>(queue_size, 1);
        auto logger = std::make_shared<async_logger>("as", test_sink, tp, async_overflow_policy::block_retry);
        for (size_t i = 0; i < messages; i++)
        {
            logger->info("Hello message #{}", i);
        }

        logger->flush();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    REQUIRE(test_sink->msg_counter() == messages);
    REQUIRE(test_sink->flushed_msg_counter() == messages);
}

TEST_CASE("multi threads", "[async]")
{
    using namespace spdlog;
    auto test_sink = std::make_shared<sinks::test_sink_mt>();
    size_t queue_size = 128;
    size_t messages = 256;
    size_t n_threads = 10;
    {
        auto tp = std::make_shared<details::thread_pool>(queue_size, 1);
        auto logger = std::make_shared<async_logger>("as", test_sink, tp, async_overflow_policy::block_retry);

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
        logger->flush();
    }

    REQUIRE(test_sink->msg_counter() == messages * n_threads);
    REQUIRE(test_sink->flushed_msg_counter() == messages * n_threads);
}

TEST_CASE("to_file", "[async]")
{
    prepare_logdir();
    size_t queue_size = 512;
    size_t messages = 512;
    size_t n_threads = 4;
    spdlog::init_thread_pool(queue_size, n_threads);
    auto logger = spdlog::basic_logger_mt<spdlog::create_async>("as", "logs/async_test.log", true);

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
    std::this_thread::sleep_for(std::chrono::seconds(1));
    REQUIRE(count_lines("logs/async_test.log") == messages * n_threads);
}
