#include "includes.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "test_sink.h"

#define TEST_FILENAME "test_logs/async_test.log"

TEST_CASE("basic async test ", "[async]")
{
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_mt>();
    size_t overrun_counter = 0;
    size_t queue_size = 128;
    size_t messages = 256;
    {
        auto tp = std::make_shared<spdlog::details::thread_pool>(queue_size, 1);
        auto logger = std::make_shared<spdlog::async_logger>("as", test_sink, tp, spdlog::async_overflow_policy::block);
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
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_mt>();
    test_sink->set_delay(std::chrono::milliseconds(1));
    size_t queue_size = 4;
    size_t messages = 1024;

    auto tp = std::make_shared<spdlog::details::thread_pool>(queue_size, 1);
    auto logger = std::make_shared<spdlog::async_logger>("as", test_sink, tp, spdlog::async_overflow_policy::overrun_oldest);
    for (size_t i = 0; i < messages; i++)
    {
        logger->info("Hello message");
    }
    REQUIRE(test_sink->msg_counter() < messages);
    REQUIRE(tp->overrun_counter() > 0);
}

TEST_CASE("discard policy using factory ", "[async]")
{
    size_t queue_size = 4;
    size_t messages = 1024;
    spdlog::init_thread_pool(queue_size, 1);

    auto logger = spdlog::create_async_nb<spdlog::sinks::test_sink_mt>("as2");
    auto test_sink = std::static_pointer_cast<spdlog::sinks::test_sink_mt>(logger->sinks()[0]);
    test_sink->set_delay(std::chrono::milliseconds(3));

    for (size_t i = 0; i < messages; i++)
    {
        logger->info("Hello message");
    }

    REQUIRE(test_sink->msg_counter() < messages);
    spdlog::drop_all();
}

TEST_CASE("flush", "[async]")
{
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_mt>();
    size_t queue_size = 256;
    size_t messages = 256;
    {
        auto tp = std::make_shared<spdlog::details::thread_pool>(queue_size, 1);
        auto logger = std::make_shared<spdlog::async_logger>("as", test_sink, tp, spdlog::async_overflow_policy::block);
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

    auto logger = spdlog::create_async<spdlog::sinks::test_sink_mt>("as");
    auto test_sink = std::static_pointer_cast<spdlog::sinks::test_sink_mt>(logger->sinks()[0]);

    spdlog::flush_every(std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(1700));
    REQUIRE(test_sink->flush_counter() == 1);
    spdlog::flush_every(std::chrono::seconds(0));
    spdlog::drop_all();
}

TEST_CASE("tp->wait_empty() ", "[async]")
{
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_mt>();
    test_sink->set_delay(std::chrono::milliseconds(5));
    size_t messages = 100;

    auto tp = std::make_shared<spdlog::details::thread_pool>(messages, 2);
    auto logger = std::make_shared<spdlog::async_logger>("as", test_sink, tp, spdlog::async_overflow_policy::block);
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
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_mt>();
    size_t queue_size = 128;
    size_t messages = 256;
    size_t n_threads = 10;
    {
        auto tp = std::make_shared<spdlog::details::thread_pool>(queue_size, 1);
        auto logger = std::make_shared<spdlog::async_logger>("as", test_sink, tp, spdlog::async_overflow_policy::block);

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
    spdlog::filename_t filename = SPDLOG_FILENAME_T(TEST_FILENAME);
    {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
        auto tp = std::make_shared<spdlog::details::thread_pool>(messages, tp_threads);
        auto logger = std::make_shared<spdlog::async_logger>("as", std::move(file_sink), std::move(tp));

        for (size_t j = 0; j < messages; j++)
        {
            logger->info("Hello message #{}", j);
        }
    }

    require_message_count(TEST_FILENAME, messages);
    auto contents = file_contents(TEST_FILENAME);
    using spdlog::details::os::default_eol;
    REQUIRE(ends_with(contents, spdlog::fmt_lib::format("Hello message #1023{}", default_eol)));
}

TEST_CASE("to_file multi-workers", "[async]")
{
    prepare_logdir();
    size_t messages = 1024 * 10;
    size_t tp_threads = 10;
    spdlog::filename_t filename = SPDLOG_FILENAME_T(TEST_FILENAME);
    {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
        auto tp = std::make_shared<spdlog::details::thread_pool>(messages, tp_threads);
        auto logger = std::make_shared<spdlog::async_logger>("as", std::move(file_sink), std::move(tp));

        for (size_t j = 0; j < messages; j++)
        {
            logger->info("Hello message #{}", j);
        }
    }
    require_message_count(TEST_FILENAME, messages);
}

TEST_CASE("bad_tp", "[async]")
{
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_mt>();
    std::shared_ptr<spdlog::details::thread_pool> const empty_tp;
    auto logger = std::make_shared<spdlog::async_logger>("as", test_sink, empty_tp);
    logger->info("Please throw an exception");
    REQUIRE(test_sink->msg_counter() == 0);
}
