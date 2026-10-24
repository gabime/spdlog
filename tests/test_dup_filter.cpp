#include "includes.h"
#include "spdlog/sinks/dup_filter_sink.h"
#include "test_sink.h"

TEST_CASE("dup_filter_construct_with_sinks", "[dup_filter_sink]") {
    auto first_sink = std::make_shared<spdlog::sinks::test_sink_mt>();
    auto second_sink = std::make_shared<spdlog::sinks::test_sink_mt>();
    spdlog::sinks::dup_filter_sink_st dup_sink{std::chrono::seconds{5}, {first_sink, second_sink}};

    REQUIRE(dup_sink.sinks().size() == 2);
    dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message"});
    dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message"});
    REQUIRE(first_sink->msg_counter() == 1);
    REQUIRE(second_sink->msg_counter() == 1);
    dup_sink.flush();
    REQUIRE(first_sink->flush_counter() == 1);
    REQUIRE(second_sink->flush_counter() == 1);
}

TEST_CASE("dup_filter_test1", "[dup_filter_sink]") {
    using spdlog::sinks::dup_filter_sink_st;
    using spdlog::sinks::test_sink_mt;

    dup_filter_sink_st dup_sink{std::chrono::seconds{5}};
    auto test_sink = std::make_shared<test_sink_mt>();
    dup_sink.add_sink(test_sink);

    for (int i = 0; i < 10; i++) {
        dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message1"});
    }

    REQUIRE(test_sink->msg_counter() == 1);
}

TEST_CASE("dup_filter_test2", "[dup_filter_sink]") {
    using spdlog::sinks::dup_filter_sink_st;
    using spdlog::sinks::test_sink_mt;

    dup_filter_sink_st dup_sink{std::chrono::seconds{0}};
    auto test_sink = std::make_shared<test_sink_mt>();
    dup_sink.add_sink(test_sink);

    for (int i = 0; i < 10; i++) {
        dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message1"});
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    REQUIRE(test_sink->msg_counter() == 10);
}

TEST_CASE("dup_filter_test3", "[dup_filter_sink]") {
    using spdlog::sinks::dup_filter_sink_st;
    using spdlog::sinks::test_sink_mt;

    dup_filter_sink_st dup_sink{std::chrono::seconds{1}};
    auto test_sink = std::make_shared<test_sink_mt>();
    dup_sink.add_sink(test_sink);

    for (int i = 0; i < 10; i++) {
        dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message1"});
        dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message2"});
    }

    REQUIRE(test_sink->msg_counter() == 20);
}

TEST_CASE("dup_filter_test4", "[dup_filter_sink]") {
    using spdlog::sinks::dup_filter_sink_mt;
    using spdlog::sinks::test_sink_mt;

    dup_filter_sink_mt dup_sink{std::chrono::milliseconds{10}};
    auto test_sink = std::make_shared<test_sink_mt>();
    dup_sink.add_sink(test_sink);

    dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message"});
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message"});
    REQUIRE(test_sink->msg_counter() == 2);
}

TEST_CASE("dup_filter_test5", "[dup_filter_sink]") {
    using spdlog::sinks::dup_filter_sink_mt;
    using spdlog::sinks::test_sink_mt;

    dup_filter_sink_mt dup_sink{std::chrono::seconds{5}};
    auto test_sink = std::make_shared<test_sink_mt>();
    test_sink->set_pattern("%v");
    dup_sink.add_sink(test_sink);

    dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message1"});
    dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message1"});
    dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message1"});
    dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message2"});

    REQUIRE(test_sink->msg_counter() ==
            3);  // skip 2 messages but log the "skipped.." message before message2
    REQUIRE(test_sink->lines()[1] == "Skipped 2 duplicate messages..");
}

TEST_CASE("dup_filter_summary_uses_event_time", "[dup_filter_sink]") {
    for (const auto asynchronous : {false, true}) {
        INFO("asynchronous: " << asynchronous);
        auto dup_sink =
            std::make_shared<spdlog::sinks::dup_filter_sink_mt>(std::chrono::seconds(5));
        auto output = std::make_shared<spdlog::sinks::test_sink_mt>();
        dup_sink->add_sink(output);
        dup_sink->set_formatter(std::unique_ptr<spdlog::formatter>(new spdlog::pattern_formatter(
            "%Y-%m-%d %H:%M:%S.%e %v", spdlog::pattern_time_type::utc)));

        std::shared_ptr<spdlog::details::thread_pool> pool;
        std::shared_ptr<spdlog::logger> logger;
        if (asynchronous) {
            pool = std::make_shared<spdlog::details::thread_pool>(16, 1);
            logger = std::make_shared<spdlog::async_logger>("test", dup_sink, pool);
        } else {
            logger = std::make_shared<spdlog::logger>("test", dup_sink);
        }

        const auto first = spdlog::log_clock::from_time_t(1);
        logger->log(first, {}, spdlog::level::info, "repeat");
        logger->log(first + std::chrono::milliseconds(100), {}, spdlog::level::info, "repeat");
        logger->log(first + std::chrono::seconds(1), {}, spdlog::level::info, "next");
        logger.reset();
        pool.reset();  // Drain the asynchronous queue before inspecting output.

        REQUIRE(output->lines() ==
                std::vector<std::string>{"1970-01-01 00:00:01.000 repeat",
                                         "1970-01-01 00:00:02.000 Skipped 1 duplicate messages..",
                                         "1970-01-01 00:00:02.000 next"});
    }
}
