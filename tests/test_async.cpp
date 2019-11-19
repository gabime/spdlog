#include "includes.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "test_sink.h"

TEST_CASE("basic async test ", "[async]")
{
    using namespace spdlog;
    auto test_sink = std::make_shared<sinks::test_sink_mt>();
    size_t overrun_counter = 0;
    size_t queue_size = 128;
    size_t messages = 256;
    {
        auto tp = std::make_shared<details::thread_pool>(queue_size, 1);
        auto logger = std::make_shared<async_logger>("as", test_sink, tp, async_overflow_policy::block);
        for (size_t i = 0; i < messages; i++)
        {
            logger->info("Hello message #{}", i);
        }
        logger->flush();
        overrun_counter = tp->overrun_counter();
    }
    REQUIRE(test_sink->msg_counter() == messages);
    REQUIRE(test_sink->flush_counter() == 1);
    REQUIRE(overrun_counter == 0);
}

TEST_CASE("discard policy ", "[async]")
{
    using namespace spdlog;
    auto test_sink = std::make_shared<sinks::test_sink_mt>();
    test_sink->set_delay(std::chrono::milliseconds(1));
    size_t queue_size = 4;
    size_t messages = 1024;

    auto tp = std::make_shared<details::thread_pool>(queue_size, 1);
    auto logger = std::make_shared<async_logger>("as", test_sink, tp, async_overflow_policy::overrun_oldest);
    for (size_t i = 0; i < messages; i++)
    {
        logger->info("Hello message");
    }
    REQUIRE(test_sink->msg_counter() < messages);
    REQUIRE(tp->overrun_counter() > 0);
}

TEST_CASE("discard policy using factory ", "[async]")
{
    using namespace spdlog;
    size_t queue_size = 4;
    size_t messages = 1024;
    spdlog::init_thread_pool(queue_size, 1);

    auto logger = spdlog::create_async_nb<sinks::test_sink_mt>("as2");
    auto test_sink = std::static_pointer_cast<sinks::test_sink_mt>(logger->sinks()[0]);
    test_sink->set_delay(std::chrono::milliseconds(1));

    for (size_t i = 0; i < messages; i++)
    {
        logger->info("Hello message");
    }

    REQUIRE(test_sink->msg_counter() < messages);
    spdlog::drop_all();
}

TEST_CASE("flush", "[async]")
{
    using namespace spdlog;
    auto test_sink = std::make_shared<sinks::test_sink_mt>();
    size_t queue_size = 256;
    size_t messages = 256;
    {
        auto tp = std::make_shared<details::thread_pool>(queue_size, 1);
        auto logger = std::make_shared<async_logger>("as", test_sink, tp, async_overflow_policy::block);
        for (size_t i = 0; i < messages; i++)
        {
            logger->info("Hello message #{}", i);
        }

        logger->flush();
    }
    // std::this_thread::sleep_for(std::chrono::milliseconds(250));
    REQUIRE(test_sink->msg_counter() == messages);
    REQUIRE(test_sink->flush_counter() == 1);
}

TEST_CASE("async periodic flush", "[async]")
{
    using namespace spdlog;

    auto logger = spdlog::create_async<sinks::test_sink_mt>("as");

    auto test_sink = std::static_pointer_cast<sinks::test_sink_mt>(logger->sinks()[0]);

    spdlog::flush_every(std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    REQUIRE(test_sink->flush_counter() == 1);
    spdlog::flush_every(std::chrono::seconds(0));
    spdlog::drop_all();
}

TEST_CASE("tp->wait_empty() ", "[async]")
{
    using namespace spdlog;
    auto test_sink = std::make_shared<sinks::test_sink_mt>();
    test_sink->set_delay(std::chrono::milliseconds(5));
    size_t messages = 100;

    auto tp = std::make_shared<details::thread_pool>(messages, 2);
    auto logger = std::make_shared<async_logger>("as", test_sink, tp, async_overflow_policy::block);
    for (size_t i = 0; i < messages; i++)
    {
        logger->info("Hello message #{}", i);
    }
    logger->flush();
    tp.reset();

    REQUIRE(test_sink->msg_counter() == messages);
    REQUIRE(test_sink->flush_counter() == 1);
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
        auto logger = std::make_shared<async_logger>("as", test_sink, tp, async_overflow_policy::block);

        std::vector<std::thread> threads;
        for (size_t i = 0; i < n_threads; i++)
        {
            threads.emplace_back([logger, messages] {
                for (size_t j = 0; j < messages; j++)
                {
                    logger->info("Hello message #{}", j);
                }
            });
            logger->flush();
        }

        for (auto &t : threads)
        {
            t.join();
        }
    }

    REQUIRE(test_sink->msg_counter() == messages * n_threads);
    REQUIRE(test_sink->flush_counter() == n_threads);
}

TEST_CASE("to_file", "[async]")
{
    prepare_logdir();
    size_t messages = 1024;
    size_t tp_threads = 1;
    std::string filename = "test_logs/async_test.log";
    {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
        auto tp = std::make_shared<spdlog::details::thread_pool>(messages, tp_threads);
        auto logger = std::make_shared<spdlog::async_logger>("as", std::move(file_sink), std::move(tp));

        for (size_t j = 0; j < messages; j++)
        {
            logger->info("Hello message #{}", j);
        }
    }

    REQUIRE(count_lines(filename) == messages);
    auto contents = file_contents(filename);
    REQUIRE(ends_with(contents, std::string("Hello message #1023\n")));
}

TEST_CASE("to_file multi-workers", "[async]")
{
    prepare_logdir();
    size_t messages = 1024 * 10;
    size_t tp_threads = 10;
    std::string filename = "test_logs/async_test.log";
    {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
        auto tp = std::make_shared<spdlog::details::thread_pool>(messages, tp_threads);
        auto logger = std::make_shared<spdlog::async_logger>("as", std::move(file_sink), std::move(tp));

        for (size_t j = 0; j < messages; j++)
        {
            logger->info("Hello message #{}", j);
        }
    }

    REQUIRE(count_lines(filename) == messages);
}
