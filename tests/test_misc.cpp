#ifdef _WIN32  // to prevent fopen warning on windows
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "includes.h"
#include "test_sink.h"

template <class T>
std::string log_info(const T& what, spdlog::level::level_enum logger_level = spdlog::level::info) {
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

    // Numbers
    REQUIRE(log_info(5) == "5");
    REQUIRE(log_info(5.6) == "5.6");

    // User defined class
    // REQUIRE(log_info(some_logged_class("some_val")) == "some_val");
}

TEST_CASE("log_levels", "[log_levels]") {
    REQUIRE(log_info("Hello", spdlog::level::err).empty());
    REQUIRE(log_info("Hello", spdlog::level::critical).empty());
    REQUIRE(log_info("Hello", spdlog::level::info) == "Hello");
    REQUIRE(log_info("Hello", spdlog::level::debug) == "Hello");
    REQUIRE(log_info("Hello", spdlog::level::trace) == "Hello");
}

TEST_CASE("level_to_string_view", "[convert_to_string_view]") {
    REQUIRE(spdlog::level::to_string_view(spdlog::level::trace) == "trace");
    REQUIRE(spdlog::level::to_string_view(spdlog::level::debug) == "debug");
    REQUIRE(spdlog::level::to_string_view(spdlog::level::info) == "info");
    REQUIRE(spdlog::level::to_string_view(spdlog::level::warn) == "warning");
    REQUIRE(spdlog::level::to_string_view(spdlog::level::err) == "error");
    REQUIRE(spdlog::level::to_string_view(spdlog::level::critical) == "critical");
    REQUIRE(spdlog::level::to_string_view(spdlog::level::off) == "off");
}

TEST_CASE("to_short_c_str", "[convert_to_short_c_str]") {
    REQUIRE(std::string(spdlog::level::to_short_c_str(spdlog::level::trace)) == "T");
    REQUIRE(std::string(spdlog::level::to_short_c_str(spdlog::level::debug)) == "D");
    REQUIRE(std::string(spdlog::level::to_short_c_str(spdlog::level::info)) == "I");
    REQUIRE(std::string(spdlog::level::to_short_c_str(spdlog::level::warn)) == "W");
    REQUIRE(std::string(spdlog::level::to_short_c_str(spdlog::level::err)) == "E");
    REQUIRE(std::string(spdlog::level::to_short_c_str(spdlog::level::critical)) == "C");
    REQUIRE(std::string(spdlog::level::to_short_c_str(spdlog::level::off)) == "O");
}

TEST_CASE("to_level_enum", "[convert_to_level_enum]") {
    REQUIRE(spdlog::level::from_str("trace") == spdlog::level::trace);
    REQUIRE(spdlog::level::from_str("debug") == spdlog::level::debug);
    REQUIRE(spdlog::level::from_str("info") == spdlog::level::info);
    REQUIRE(spdlog::level::from_str("warning") == spdlog::level::warn);
    REQUIRE(spdlog::level::from_str("warn") == spdlog::level::warn);
    REQUIRE(spdlog::level::from_str("error") == spdlog::level::err);
    REQUIRE(spdlog::level::from_str("critical") == spdlog::level::critical);
    REQUIRE(spdlog::level::from_str("off") == spdlog::level::off);
    REQUIRE(spdlog::level::from_str("null") == spdlog::level::off);
    REQUIRE(spdlog::level::from_str("TRACE") == spdlog::level::trace);
    REQUIRE(spdlog::level::from_str("DEBUG") == spdlog::level::debug);
    REQUIRE(spdlog::level::from_str("INFO") == spdlog::level::info);
    REQUIRE(spdlog::level::from_str("WARNING") == spdlog::level::warn);
    REQUIRE(spdlog::level::from_str("WARN") == spdlog::level::warn);
    REQUIRE(spdlog::level::from_str("ERROR") == spdlog::level::err);
    REQUIRE(spdlog::level::from_str("ERR") == spdlog::level::err);
    REQUIRE(spdlog::level::from_str("CRITICAL") == spdlog::level::critical);
    REQUIRE(spdlog::level::from_str("OFF") == spdlog::level::off);
    REQUIRE(spdlog::level::from_str("TrAcE") == spdlog::level::trace);
    REQUIRE(spdlog::level::from_str("DeBuG") == spdlog::level::debug);
}

TEST_CASE("periodic flush", "[periodic_flush]") {
    using spdlog::sinks::test_sink_mt;
    auto logger = spdlog::create<test_sink_mt>("periodic_flush");
    auto test_sink = std::static_pointer_cast<test_sink_mt>(logger->sinks()[0]);

    spdlog::flush_every(std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(1250));
    REQUIRE(test_sink->flush_counter() == 1);
    spdlog::flush_every(std::chrono::seconds(0));
    spdlog::drop_all();
}

TEST_CASE("clone-logger", "[clone]") {
    using spdlog::sinks::test_sink_mt;
    auto test_sink = std::make_shared<test_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("orig", test_sink);
    logger->set_pattern("%v");
    auto cloned = logger->clone("clone");

    REQUIRE(cloned->name() == "clone");
    REQUIRE(logger->sinks() == cloned->sinks());
    REQUIRE(logger->level() == cloned->level());
    REQUIRE(logger->flush_level() == cloned->flush_level());
    logger->info("Some message 1");
    cloned->info("Some message 2");

    REQUIRE(test_sink->lines().size() == 2);
    REQUIRE(test_sink->lines()[0] == "Some message 1");
    REQUIRE(test_sink->lines()[1] == "Some message 2");

    spdlog::drop_all();
}

TEST_CASE("clone async", "[clone]") {
    using spdlog::sinks::test_sink_mt;
    spdlog::init_thread_pool(4, 1);
    auto test_sink = std::make_shared<test_sink_mt>();
    auto logger = std::make_shared<spdlog::async_logger>("orig", test_sink, spdlog::thread_pool());
    logger->set_pattern("%v");
    auto cloned = logger->clone("clone");

    REQUIRE(cloned->name() == "clone");
    REQUIRE(logger->sinks() == cloned->sinks());
    REQUIRE(logger->level() == cloned->level());
    REQUIRE(logger->flush_level() == cloned->flush_level());

    logger->info("Some message 1");
    cloned->info("Some message 2");

    spdlog::details::os::sleep_for_millis(100);

    REQUIRE(test_sink->lines().size() == 2);
    REQUIRE(test_sink->lines()[0] == "Some message 1");
    REQUIRE(test_sink->lines()[1] == "Some message 2");

    spdlog::drop_all();
}

TEST_CASE("default logger API", "[default logger]") {
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

    spdlog::set_default_logger(std::make_shared<spdlog::logger>("oss", oss_sink));
    spdlog::set_pattern("*** %v");

    spdlog::default_logger()->set_level(spdlog::level::trace);
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
    spdlog::error(123);
    REQUIRE(oss.str() == "*** 123" + std::string(spdlog::details::os::default_eol));

    oss.str("");
    spdlog::critical(std::string("some string"));
    REQUIRE(oss.str() == "*** some string" + std::string(spdlog::details::os::default_eol));

    oss.str("");
    spdlog::set_level(spdlog::level::info);
    spdlog::debug("should not be logged");
    REQUIRE(oss.str().empty());
    spdlog::drop_all();
    spdlog::set_pattern("%v");
}

#if (defined(SPDLOG_WCHAR_TO_UTF8_SUPPORT) || defined(SPDLOG_WCHAR_FILENAMES)) && defined(_WIN32)
TEST_CASE("utf8 to utf16 conversion using windows api", "[windows utf]") {
    spdlog::wmemory_buf_t buffer;

    spdlog::details::os::utf8_to_wstrbuf("", buffer);
    REQUIRE(std::wstring(buffer.data(), buffer.size()) == std::wstring(L""));

    spdlog::details::os::utf8_to_wstrbuf("abc", buffer);
    REQUIRE(std::wstring(buffer.data(), buffer.size()) == std::wstring(L"abc"));

    spdlog::details::os::utf8_to_wstrbuf("\xc3\x28", buffer);  // Invalid UTF-8 sequence.
    REQUIRE(std::wstring(buffer.data(), buffer.size()) == std::wstring(L"\xfffd("));

    spdlog::details::os::utf8_to_wstrbuf("\xe3\x81\xad\xe3\x81\x93",
                                         buffer);  // "Neko" in hiragana.
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
    REQUIRE(create_dir(SPDLOG_FILENAME_T("log_tests")) == true);
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
