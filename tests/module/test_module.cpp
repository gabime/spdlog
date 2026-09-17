// Tests for the C++20 named module.
//
// This is a separate executable from spdlog-utests: the shared test sources reach
// spdlog through tests/includes.h, i.e. #include, and a translation unit cannot
// sensibly do both for the same library.
//
// Beyond re-checking a slice of the API through `import spdlog;`, this covers the
// three properties that are specific to the module and have no other coverage:
//   1. the fmt::formatter specializations compiled into the module's purview
//      (not exported themselves, but reachable through the exported primary
//      fmt::formatter template) survive and are usable (stopwatch, to_hex),
//   2. the re-exported fmt names are enough for a consumer to specialize
//      fmt::formatter<T> itself,
//   3. <spdlog/macros.h> works with no spdlog header included at all.
//
// All #includes precede the import: MSVC rejects a standard header included after
// importing a module whose global module fragment already had it.
#include <catch2/catch_test_macros.hpp>

// <chrono>: pattern_formatter's default constructor (used below) can implicitly
// instantiate details::elapsed_formatter<Padder, std::chrono::seconds> in this TU; MSVC's
// module implementation needs std::chrono::duration's comparison operators to be visible
// here directly, not just reachable through spdlog.cppm's global module fragment. See
// tests/cmake/module/main.cpp for the long version of this explanation.
#include <chrono>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include <spdlog/macros.h>

// The fmt names needed to specialize fmt::formatter<T> are re-exported by the module.
// Do NOT #include a header that declares spdlog's *own* entities (e.g. <spdlog/mdc.h>) here:
// this native module attaches those to the named module `spdlog`, so #including one on top
// of importing creates a second, distinct copy of the same names (ODR violation - observed
// as a duplicate-symbol link error with Clang/lld). See INSTALL for the full explanation.
#if defined(SPDLOG_USE_STD_FORMAT)
#include <format>  // std::formatter; names from namespace std are never re-exported
#endif

import spdlog;

struct my_type {
    int i = 0;
};

#if !defined(SPDLOG_USE_STD_FORMAT)
template <>
struct fmt::formatter<my_type> : fmt::formatter<std::string> {
    auto format(my_type m, fmt::format_context &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "[my_type i={}]", m.i);
    }
};
#else
template <>
struct std::formatter<my_type> : std::formatter<std::string> {
    auto format(my_type m, std::format_context &ctx) const -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "[my_type i={}]", m.i);
    }
};
#endif

// Capture into an ostringstream through an ostream_sink, so the assertions can
// look at real formatted output.
static std::string log_to_string(const std::function<void(spdlog::logger &)> &fn,
                                 const std::string &pattern = "%v") {
    std::ostringstream oss;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    spdlog::logger logger("module_test", sink);
    logger.set_level(spdlog::level::trace);
    logger.set_pattern(pattern);
    fn(logger);
    logger.flush();
    return oss.str();
}

TEST_CASE("module: basic formatting", "[module]") {
    auto out = log_to_string([](spdlog::logger &l) { l.info("hello {} {}", 1, "two"); });
    REQUIRE(out.find("hello 1 two") != std::string::npos);
}

TEST_CASE("module: levels are exported with their enumerators", "[module]") {
    REQUIRE(spdlog::level::to_string_view(spdlog::level::info) == std::string("info"));
    REQUIRE(spdlog::level::from_str("warning") == spdlog::level::warn);

    spdlog::logger logger("lvl", std::make_shared<spdlog::sinks::null_sink_mt>());
    logger.set_level(spdlog::level::err);
    REQUIRE(logger.should_log(spdlog::level::critical));
    REQUIRE_FALSE(logger.should_log(spdlog::level::info));
}

TEST_CASE("module: consumer-defined formatter specialization", "[module]") {
    auto out = log_to_string([](spdlog::logger &l) { l.info("{}", my_type{7}); });
    REQUIRE(out.find("[my_type i=7]") != std::string::npos);
}

TEST_CASE("module: stopwatch formatter survives inside the module's purview", "[module]") {
    spdlog::stopwatch sw;
    auto out = log_to_string([&sw](spdlog::logger &l) { l.info("{:.3}", sw); });
    // just needs to have formatted as a number rather than failing to compile
    REQUIRE(out.size() > 1);
}

#if !defined(SPDLOG_USE_STD_FORMAT) || defined(_MSC_VER)
TEST_CASE("module: to_hex formatter survives inside the module's purview", "[module]") {
    std::vector<unsigned char> buf{0x00, 0x01, 0x02, 0x0a};
    auto out = log_to_string([&buf](spdlog::logger &l) { l.info("{:n}", spdlog::to_hex(buf)); });
    REQUIRE(out.find("00 01 02 0a") != std::string::npos);
}
#endif

TEST_CASE("module: macros work with no spdlog header included", "[module]") {
    // SPDLOG_ACTIVE_LEVEL is debug, so the debug macro must expand to a real call
    auto out = log_to_string([](spdlog::logger &l) {
        SPDLOG_LOGGER_DEBUG(&l, "macro debug {}", 42);
        SPDLOG_LOGGER_TRACE(&l, "macro trace should be compiled out");
    });
    REQUIRE(out.find("macro debug 42") != std::string::npos);
    REQUIRE(out.find("compiled out") == std::string::npos);
}

TEST_CASE("module: source location through the macros", "[module]") {
    auto out =
        log_to_string([](spdlog::logger &l) { SPDLOG_LOGGER_INFO(&l, "located"); }, "%s:%# %v");
    REQUIRE(out.find("test_module.cpp") != std::string::npos);
}

TEST_CASE("module: registry functions", "[module]") {
    spdlog::drop("registered_by_module");
    auto logger = spdlog::null_logger_mt("registered_by_module");
    REQUIRE(spdlog::get("registered_by_module") != nullptr);
    spdlog::drop("registered_by_module");
    REQUIRE(spdlog::get("registered_by_module") == nullptr);
}

TEST_CASE("module: async factory", "[module]") {
    // references details::default_async_q_size, compiled into the module's purview
    spdlog::drop("async_module_logger");
    auto logger = spdlog::null_logger_mt<spdlog::async_factory>("async_module_logger");
    logger->info("async through the module");
    logger->flush();
    spdlog::drop("async_module_logger");
}

TEST_CASE("module: pattern_formatter and make_unique", "[module]") {
    // the pattern_formatter default arguments reference details::os::default_eol
    auto formatter = spdlog::details::make_unique<spdlog::pattern_formatter>();
    spdlog::memory_buf_t dest;
    spdlog::details::log_msg msg("logger_name", spdlog::level::info, "some message");
    formatter->format(msg, dest);
    REQUIRE(std::string(dest.data(), dest.size()).find("some message") != std::string::npos);
}

TEST_CASE("module: exceptions", "[module]") {
    REQUIRE_THROWS_AS(spdlog::throw_spdlog_ex("boom"), spdlog::spdlog_ex);
}

#ifndef SPDLOG_NO_TLS
TEST_CASE("module: mdc", "[module]") {
    spdlog::mdc::clear();
    spdlog::mdc::put("mdc_key", "mdc_value");
    auto out = log_to_string([](spdlog::logger &l) { l.info("with context"); }, "[%&] %v");
    REQUIRE(out.find("mdc_key:mdc_value") != std::string::npos);
    spdlog::mdc::clear();
}
#endif
