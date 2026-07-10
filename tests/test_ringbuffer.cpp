#include "includes.h"
#include "spdlog/sinks/ringbuffer_sink.h"

TEST_CASE("ringbuffer invalid size", "[ringbuffer]") {
    REQUIRE_THROWS_AS(spdlog::sinks::ringbuffer_sink_mt(0), spdlog::spdlog_ex);
}

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
