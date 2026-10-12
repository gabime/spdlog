#include "includes.h"
#include "spdlog/sinks/ringbuffer_sink.h"

#ifndef SPDLOG_NO_EXCEPTIONS
TEST_CASE("ringbuffer invalid size", "[ringbuffer]") {
    REQUIRE_THROWS_AS(spdlog::sinks::ringbuffer_sink_mt(0), spdlog::spdlog_ex);
}
#endif  // SPDLOG_NO_EXCEPTIONS

TEST_CASE("ringbuffer stores formatted messages", "[ringbuffer]") {
    spdlog::sinks::ringbuffer_sink_st sink(3);
    sink.set_pattern("%v");

    sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "msg1"});
    sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "msg2"});
    sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "msg3"});

    auto formatted = sink.last_formatted();
    REQUIRE(formatted.size() == 3);
    using spdlog::details::os::default_eol;
    REQUIRE(formatted[0] == spdlog::fmt_lib::format("msg1{}", default_eol));
    REQUIRE(formatted[1] == spdlog::fmt_lib::format("msg2{}", default_eol));
    REQUIRE(formatted[2] == spdlog::fmt_lib::format("msg3{}", default_eol));
}

TEST_CASE("ringbuffer overrun keeps last items", "[ringbuffer]") {
    spdlog::sinks::ringbuffer_sink_st sink(2);
    sink.set_pattern("%v");

    sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "first"});
    sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "second"});
    sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "third"});

    auto formatted = sink.last_formatted();
    REQUIRE(formatted.size() == 2);
    using spdlog::details::os::default_eol;
    REQUIRE(formatted[0] == spdlog::fmt_lib::format("second{}", default_eol));
    REQUIRE(formatted[1] == spdlog::fmt_lib::format("third{}", default_eol));
}

TEST_CASE("ringbuffer retrieval limit", "[ringbuffer]") {
    spdlog::sinks::ringbuffer_sink_st sink(3);
    sink.set_pattern("%v");

    sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "A"});
    sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "B"});
    sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "C"});

    auto formatted = sink.last_formatted(2);
    REQUIRE(formatted.size() == 2);
    using spdlog::details::os::default_eol;
    REQUIRE(formatted[0] == spdlog::fmt_lib::format("B{}", default_eol));
    REQUIRE(formatted[1] == spdlog::fmt_lib::format("C{}", default_eol));
}

// Regression test for #3195: source_loc filename/funcname must be buffered by
// log_msg_buffer, otherwise they dangle once the caller's strings are gone.
TEST_CASE("ringbuffer buffers source_loc strings", "[ringbuffer]") {
    spdlog::sinks::ringbuffer_sink_st sink(2);
    sink.set_pattern("%g:%# %! %v");

    {
        // filename/funcname from scoped strings that are destroyed before retrieval
        std::string filename = std::string("dynamic_") + "source.cpp";
        std::string funcname = std::string("dynamic_") + "function";
        spdlog::source_loc loc{filename.c_str(), 123, funcname.c_str()};
        sink.log(spdlog::details::log_msg{loc, "test", spdlog::level::info, "payload"});
    }

    // overwrite the freed stack memory
    volatile char scratch[256];
    for (size_t i = 0; i < sizeof(scratch); ++i) scratch[i] = static_cast<char>(i);

    auto formatted = sink.last_formatted();
    REQUIRE(formatted.size() == 1);
    using spdlog::details::os::default_eol;
    REQUIRE(formatted[0] ==
            spdlog::fmt_lib::format("dynamic_source.cpp:123 dynamic_function payload{}",
                                    default_eol));
}

TEST_CASE("ringbuffer raw messages survive copy self-assignment", "[ringbuffer]") {
    const auto payload_size = GENERATE(size_t{8}, size_t{1024});
    const std::string payload(payload_size, 'x');
    auto sink = std::make_shared<spdlog::sinks::ringbuffer_sink_st>(2);
    spdlog::logger logger("raw-copies", sink);
    logger.warn("{}", payload);
    logger.info("{}", std::string(payload_size, 'z'));

    auto messages = sink->last_raw();
    REQUIRE(messages.size() == 2);
    const auto time = messages.front().time;
    const auto thread_id = messages.front().thread_id;
    const auto& source = messages.front();
    messages.front() = source;
    messages.back() = source;

    CHECK(std::string(messages.back().payload.data(), messages.back().payload.size()) == payload);
    CHECK(std::string(messages.back().logger_name.data(), messages.back().logger_name.size()) ==
          "raw-copies");
    CHECK(messages.back().level == spdlog::level::warn);
    CHECK(messages.back().time == time);
    CHECK(messages.back().thread_id == thread_id);
}

TEST_CASE("ringbuffer raw message copy and move assignment own source strings", "[ringbuffer]") {
    spdlog::sinks::ringbuffer_sink_st sink(1);
    {
        std::string filename = "copy-source.cpp";
        std::string funcname = "copy_function";
        std::string payload(1024, 'x');
        sink.log(
            spdlog::details::log_msg{spdlog::source_loc{filename.c_str(), 42, funcname.c_str()},
                                     "source-copies", spdlog::level::info, payload});
    }

    auto messages = sink.last_raw();
    REQUIRE(messages.size() == 1);
    spdlog::details::log_msg_buffer copied;
    copied = messages.front();
    spdlog::details::log_msg_buffer constructed(copied);
    spdlog::details::log_msg_buffer moved(std::move(constructed));
    spdlog::details::log_msg_buffer assigned;
    assigned = std::move(moved);
    messages.clear();
    copied = spdlog::details::log_msg_buffer{};

    CHECK(std::string(assigned.payload.data(), assigned.payload.size()) == std::string(1024, 'x'));
    CHECK(std::string(assigned.logger_name.data(), assigned.logger_name.size()) == "source-copies");
    CHECK(std::string(assigned.source.filename) == "copy-source.cpp");
    CHECK(std::string(assigned.source.funcname) == "copy_function");
    CHECK(assigned.source.line == 42);
}
