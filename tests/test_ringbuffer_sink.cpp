#include "includes.h"
#include "spdlog/sinks/ringbuffer_sink.h"

TEST_CASE("test_drain", "[ringbuffer_sink]")
{
    const size_t sink_size = 3;
    auto sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(sink_size);
    spdlog::logger l("logger", sink);
    l.set_pattern("*** %v");

    // log more than the sink size by one and test that the first message is dropped
    // test 3 times to make sure the ringbuffer is working correctly multiple times
    for (int i = 0; i < 3; i++)
    {
        for (size_t i = 0; i < sink_size + 1; ++i)
        {
            l.info("{}", i);
        }

        int counter = 0;
        sink->drain([&](std::string_view msg) {
            REQUIRE(msg == fmt::format("*** {}{}", counter + 1, spdlog::details::os::default_eol));
            counter++;
        });

        REQUIRE(counter == sink_size);
    }
}

TEST_CASE("test_drain_raw", "[ringbuffer_sink]")
{
    const size_t sink_size = 3;
    auto sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(sink_size);
    spdlog::logger l("logger", sink);

    // log more than the sink size by one and test that the first message is dropped
    // test 3 times to make sure the ringbuffer is working correctly multiple times
    for (int i = 0; i < 3; i++)
    {
        for (size_t i = 0; i < sink_size + 1; ++i)
        {
            l.info("{}", i);
        }

        int counter = 0;
        sink->drain_raw([&](const spdlog::details::log_msg_buffer &buffer) {
            REQUIRE(buffer.payload.data() == std::to_string(counter + 1));
            counter++;
        });

        REQUIRE(counter == sink_size);
    }
}

TEST_CASE("test_empty", "[ringbuffer_sink]")
{
    const size_t sink_size = 3;
    auto sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(sink_size);
    spdlog::logger l("logger", sink);

    sink->drain([&](std::string_view msg) {
        REQUIRE_FALSE(true); // should not be called since the sink is empty
    });
}

TEST_CASE("test_empty_size", "[ringbuffer_sink]")
{
    const size_t sink_size = 0;
    auto sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(sink_size);
    spdlog::logger l("logger", sink);

    for (size_t i = 0; i < sink_size + 1; ++i)
    {
        l.info("{}", i);
    }

    sink->drain([&](std::string_view msg) {
        REQUIRE_FALSE(true); // should not be called since the sink size is 0
    });
}