#ifdef _WIN32  // to prevent fopen warning on windows
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdio>
#include <cstring>

#include "includes.h"
#include "spdlog/details/os.h"
#include "spdlog/sinks/async_sink.h"
#include "spdlog/sinks/ostream_sink.h"
#include "test_sink.h"

template <class T>
std::string log_info(const T& what, spdlog::level logger_level = spdlog::level::info) {
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

    spdlog::logger oss_logger("oss", oss_sink);
    oss_logger.set_level(logger_level);
    oss_logger.set_pattern("%v");
    oss_logger.info(what);

    return oss.str().substr(0, oss.str().length() - strlen(spdlog::details::os::default_eol));
}

TEST_CASE("basic_logging ", "[basic_logging]") {
    // const char
    REQUIRE(log_info("Hello") == "Hello");
    REQUIRE(log_info("").empty());

    // std::string
    REQUIRE(log_info(std::string("Hello")) == "Hello");
    REQUIRE(log_info(std::string()).empty());
}

TEST_CASE("log_levels", "[log_levels]") {
    REQUIRE(log_info("Hello", spdlog::level::err).empty());
    REQUIRE(log_info("Hello", spdlog::level::critical).empty());
    REQUIRE(log_info("Hello", spdlog::level::info) == "Hello");
    REQUIRE(log_info("Hello", spdlog::level::debug) == "Hello");
    REQUIRE(log_info("Hello", spdlog::level::trace) == "Hello");
}

TEST_CASE("level_to_string_view", "[convert_to_string_view]") {
    REQUIRE(spdlog::to_string_view(spdlog::level::trace) == "trace");
    REQUIRE(spdlog::to_string_view(spdlog::level::debug) == "debug");
    REQUIRE(spdlog::to_string_view(spdlog::level::info) == "info");
    REQUIRE(spdlog::to_string_view(spdlog::level::warn) == "warning");
    REQUIRE(spdlog::to_string_view(spdlog::level::err) == "error");
    REQUIRE(spdlog::to_string_view(spdlog::level::critical) == "critical");
    REQUIRE(spdlog::to_string_view(spdlog::level::off) == "off");
}

TEST_CASE("to_short_string_view", "[convert_to_short_c_str]") {
    REQUIRE(spdlog::to_short_string_view(spdlog::level::trace) == "T");
    REQUIRE(spdlog::to_short_string_view(spdlog::level::debug) == "D");
    REQUIRE(spdlog::to_short_string_view(spdlog::level::info) == "I");
    REQUIRE(spdlog::to_short_string_view(spdlog::level::warn) == "W");
    REQUIRE(spdlog::to_short_string_view(spdlog::level::err) == "E");
    REQUIRE(spdlog::to_short_string_view(spdlog::level::critical) == "C");
    REQUIRE(spdlog::to_short_string_view(spdlog::level::off) == "O");
}

TEST_CASE("to_level_enum", "[convert_to_level_enum]") {
    REQUIRE(spdlog::level_from_str("trace") == spdlog::level::trace);
    REQUIRE(spdlog::level_from_str("debug") == spdlog::level::debug);
    REQUIRE(spdlog::level_from_str("info") == spdlog::level::info);
    REQUIRE(spdlog::level_from_str("warning") == spdlog::level::warn);
    REQUIRE(spdlog::level_from_str("warn") == spdlog::level::warn);
    REQUIRE(spdlog::level_from_str("error") == spdlog::level::err);
    REQUIRE(spdlog::level_from_str("critical") == spdlog::level::critical);
    REQUIRE(spdlog::level_from_str("off") == spdlog::level::off);
    REQUIRE(spdlog::level_from_str("null") == spdlog::level::off);
    REQUIRE(spdlog::level_from_str("TRACE") == spdlog::level::trace);
    REQUIRE(spdlog::level_from_str("DEBUG") == spdlog::level::debug);
    REQUIRE(spdlog::level_from_str("INFO") == spdlog::level::info);
    REQUIRE(spdlog::level_from_str("WARNING") == spdlog::level::warn);
    REQUIRE(spdlog::level_from_str("WARN") == spdlog::level::warn);
    REQUIRE(spdlog::level_from_str("ERROR") == spdlog::level::err);
    REQUIRE(spdlog::level_from_str("ERR") == spdlog::level::err);
    REQUIRE(spdlog::level_from_str("CRITICAL") == spdlog::level::critical);
    REQUIRE(spdlog::level_from_str("OFF") == spdlog::level::off);
    REQUIRE(spdlog::level_from_str("TrAcE") == spdlog::level::trace);
    REQUIRE(spdlog::level_from_str("DeBuG") == spdlog::level::debug);
}

TEST_CASE("copy_ctor", "[copy_ctor]") {
    using spdlog::sinks::test_sink_mt;
    auto test_sink = std::make_shared<test_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("orig", test_sink);
    logger->set_pattern("%v");
    bool error_handled = false;
    logger->set_error_handler([&error_handled](const std::string&) { error_handled = true; });
    spdlog::logger copied = *logger;

    REQUIRE(copied.name() == logger->name());
    REQUIRE(logger->sinks() == copied.sinks());
    REQUIRE(logger->log_level() == copied.log_level());
    REQUIRE(logger->flush_level() == copied.flush_level());

    logger->info("Some message 1");
    copied.info("Some message 2");

    REQUIRE(test_sink->lines().size() == 2);
    REQUIRE(test_sink->lines()[0] == "Some message 1");
    REQUIRE(test_sink->lines()[1] == "Some message 2");

    // check that copied custom error handler was indeed copied
    test_sink->set_exception(std::runtime_error("Some error"));
    REQUIRE(error_handled == false);
    copied.error("Some error");
    REQUIRE(error_handled == true);
}

TEST_CASE("move_ctor", "[move_ctor]") {
    auto log_level = spdlog::level::critical;
    auto flush_level = spdlog::level::warn;
    using spdlog::sinks::test_sink_mt;
    auto test_sink = std::make_shared<test_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("orig", test_sink);
    logger->flush_on(flush_level);
    logger->set_level(log_level);
    logger->set_pattern("%v");
    bool error_handled = false;
    logger->set_error_handler([&error_handled](const std::string&) { error_handled = true; });
    spdlog::logger moved = std::move(*logger);

    REQUIRE(logger->name() == "");
    REQUIRE(logger->sinks().empty());
    REQUIRE(moved.name() == "orig");
    REQUIRE(moved.sinks()[0].get() == test_sink.get());
    REQUIRE(moved.log_level() == log_level);
    REQUIRE(moved.flush_level() == flush_level);

    logger->critical("Some message 1");
    moved.critical("Some message 2");

    REQUIRE(test_sink->lines().size() == 1);
    REQUIRE(test_sink->lines()[0] == "Some message 2");

    // check that copied custom error handler was indeed copied
    test_sink->set_exception(std::runtime_error("Some error"));
    REQUIRE(error_handled == false);
    moved.critical("Some error");
    REQUIRE(error_handled == true);
}

TEST_CASE("clone-logger", "[clone]") {
    using spdlog::sinks::test_sink_mt;
    auto test_sink = std::make_shared<test_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("orig", test_sink);
    logger->set_pattern("%v");
    bool error_handled = false;
    logger->set_error_handler([&error_handled](const std::string&) { error_handled = true; });
    auto cloned = logger->clone("clone");

    REQUIRE(cloned->name() == "clone");
    REQUIRE(logger->sinks() == cloned->sinks());
    REQUIRE(logger->log_level() == cloned->log_level());
    REQUIRE(logger->flush_level() == cloned->flush_level());

    logger->info("Some message 1");
    cloned->info("Some message 2");

    REQUIRE(test_sink->lines().size() == 2);
    REQUIRE(test_sink->lines()[0] == "Some message 1");
    REQUIRE(test_sink->lines()[1] == "Some message 2");

    // check that cloned custom error handler was indeed cloned
    test_sink->set_exception(std::runtime_error("Some error"));
    REQUIRE(error_handled == false);
    cloned->error("Some error");
    REQUIRE(error_handled == true);
}

TEST_CASE("clone async", "[clone]") {
    using spdlog::sinks::test_sink_mt;
    auto test_sink = std::make_shared<test_sink_mt>();
    {
        auto cfg = spdlog::sinks::async_sink::config();
        cfg.sinks.push_back(test_sink);
        auto async_sink = spdlog::sinks::async_sink::with<test_sink_mt>();
        auto logger = spdlog::create<spdlog::sinks::async_sink>("orig", cfg);
        logger->set_pattern("*** %v ***");
        auto cloned = logger->clone("clone");
        REQUIRE(cloned->name() == "clone");
        REQUIRE(logger->sinks() == cloned->sinks());
        REQUIRE(logger->log_level() == cloned->log_level());
        REQUIRE(logger->flush_level() == cloned->flush_level());

        logger->info("Some message 1");
        cloned->info("Some message 2");
    }
    REQUIRE(test_sink->lines().size() == 2);
    REQUIRE(test_sink->lines()[0] == "*** Some message 1 ***");
    REQUIRE(test_sink->lines()[1] == "*** Some message 2 ***");
}

TEST_CASE("global logger API", "[global logger]") {
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

    spdlog::set_global_logger(std::make_shared<spdlog::logger>("oss", oss_sink));
    spdlog::set_pattern("*** %v");

    spdlog::global_logger()->set_level(spdlog::level::trace);
    spdlog::trace("hello trace");
    REQUIRE(oss.str() == "*** hello trace" + std::string(spdlog::details::os::default_eol));

    oss.str("");
    spdlog::debug("hello debug");
    REQUIRE(oss.str() == "*** hello debug" + std::string(spdlog::details::os::default_eol));

    oss.str("");
    spdlog::info("Hello");
    REQUIRE(oss.str() == "*** Hello" + std::string(spdlog::details::os::default_eol));

    oss.str("");
    spdlog::warn("Hello again {}", 2);
    REQUIRE(oss.str() == "*** Hello again 2" + std::string(spdlog::details::os::default_eol));

    oss.str("");
    spdlog::critical(std::string("some string"));
    REQUIRE(oss.str() == "*** some string" + std::string(spdlog::details::os::default_eol));

    oss.str("");
    spdlog::set_level(spdlog::level::info);
    spdlog::debug("should not be logged");
    REQUIRE(oss.str().empty());
    spdlog::set_pattern("%v");
}

#if defined(_WIN32)
TEST_CASE("utf8 to utf16 conversion using windows api", "[windows utf]") {
    spdlog::wmemory_buf_t buffer;

    spdlog::details::os::utf8_to_wstrbuf("", buffer);
    REQUIRE(std::wstring(buffer.data(), buffer.size()) == std::wstring(L""));

    spdlog::details::os::utf8_to_wstrbuf("abc", buffer);
    REQUIRE(std::wstring(buffer.data(), buffer.size()) == std::wstring(L"abc"));

    spdlog::details::os::utf8_to_wstrbuf("\xc3\x28", buffer);  // Invalid UTF-8 sequence.
    REQUIRE(std::wstring(buffer.data(), buffer.size()) == std::wstring(L"\xfffd("));

    spdlog::details::os::utf8_to_wstrbuf("\xe3\x81\xad\xe3\x81\x93", buffer);  // "Neko" in hiragana.
    REQUIRE(std::wstring(buffer.data(), buffer.size()) == std::wstring(L"\x306d\x3053"));
}
#endif

struct auto_closer {
    FILE* fp = nullptr;
    explicit auto_closer(FILE* f)
        : fp(f) {}
    auto_closer(const auto_closer&) = delete;
    auto_closer& operator=(const auto_closer&) = delete;
    ~auto_closer() {
        if (fp != nullptr) (void)std::fclose(fp);
    }
};

TEST_CASE("os::fwrite_bytes", "[os]") {
    using spdlog::details::os::create_dir;
    using spdlog::details::os::fwrite_bytes;
    const char* filename = "log_tests/test_fwrite_bytes.txt";
    const char* msg = "hello";
    prepare_logdir();
    REQUIRE(create_dir("log_tests"));
    {
        auto_closer closer(std::fopen(filename, "wb"));
        REQUIRE(closer.fp != nullptr);
        REQUIRE(fwrite_bytes(msg, std::strlen(msg), closer.fp) == true);
        REQUIRE(fwrite_bytes(msg, 0, closer.fp) == true);
        std::fflush(closer.fp);
        REQUIRE(spdlog::details::os::filesize(closer.fp) == 5);
    }
    // fwrite_bytes should return false on write failure
    auto_closer closer(std::fopen(filename, "r"));
    REQUIRE(closer.fp != nullptr);
    REQUIRE_FALSE(fwrite_bytes("Hello", 5, closer.fp));
}
