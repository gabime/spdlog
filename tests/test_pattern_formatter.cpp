#include "includes.h"

// log to str and return it
template<typename... Args>

static std::string log_to_str(const std::string &msg, const Args &... args)
{
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    spdlog::logger oss_logger("pattern_tester", oss_sink);
    oss_logger.set_level(spdlog::level::info);

    oss_logger.set_formatter(std::unique_ptr<spdlog::formatter>(new spdlog::pattern_formatter(args...)));

    oss_logger.info(msg);
    return oss.str();
}

TEST_CASE("custom eol", "[pattern_formatter]")
{
    std::string msg = "Hello custom eol test";
    std::string eol = ";)";
    REQUIRE(log_to_str(msg, "%v", spdlog::pattern_time_type::local, ";)") == msg + eol);
}

TEST_CASE("empty format", "[pattern_formatter]")
{
    REQUIRE(log_to_str("Some message", "", spdlog::pattern_time_type::local, "") == "");
}

TEST_CASE("empty format2", "[pattern_formatter]")
{
    REQUIRE(log_to_str("Some message", "", spdlog::pattern_time_type::local, "\n") == "\n");
}

TEST_CASE("level", "[pattern_formatter]")
{
    REQUIRE(log_to_str("Some message", "[%l] %v", spdlog::pattern_time_type::local, "\n") == "[info] Some message\n");
}

TEST_CASE("short level", "[pattern_formatter]")
{
    REQUIRE(log_to_str("Some message", "[%L] %v", spdlog::pattern_time_type::local, "\n") == "[I] Some message\n");
}

TEST_CASE("name", "[pattern_formatter]")
{
    REQUIRE(log_to_str("Some message", "[%n] %v", spdlog::pattern_time_type::local, "\n") == "[pattern_tester] Some message\n");
}

TEST_CASE("date MM/DD/YY ", "[pattern_formatter]")
{
    auto now_tm = spdlog::details::os::localtime();
    std::stringstream oss;
    oss << std::setfill('0') << std::setw(2) << now_tm.tm_mon + 1 << "/" << std::setw(2) << now_tm.tm_mday << "/" << std::setw(2)
        << (now_tm.tm_year + 1900) % 1000 << " Some message\n";
    REQUIRE(log_to_str("Some message", "%D %v", spdlog::pattern_time_type::local, "\n") == oss.str());
}

TEST_CASE("color range test1", "[pattern_formatter]")
{
    auto formatter = std::make_shared<spdlog::pattern_formatter>("%^%v%$", spdlog::pattern_time_type::local, "\n");

    fmt::memory_buffer buf;
    fmt::format_to(buf, "Hello");
    fmt::memory_buffer formatted;
    std::string logger_name = "test";
    spdlog::details::log_msg msg(&logger_name, spdlog::level::info, spdlog::string_view_t(buf.data(), buf.size()));
    formatter->format(msg, formatted);
    REQUIRE(msg.color_range_start == 0);
    REQUIRE(msg.color_range_end == 5);
    REQUIRE(log_to_str("hello", "%^%v%$", spdlog::pattern_time_type::local, "\n") == "hello\n");
}

TEST_CASE("color range test2", "[pattern_formatter]")
{
    auto formatter = std::make_shared<spdlog::pattern_formatter>("%^%$", spdlog::pattern_time_type::local, "\n");
    std::string logger_name = "test";
    spdlog::details::log_msg msg(&logger_name, spdlog::level::info, "");
    fmt::memory_buffer formatted;
    formatter->format(msg, formatted);
    REQUIRE(msg.color_range_start == 0);
    REQUIRE(msg.color_range_end == 0);
    REQUIRE(log_to_str("", "%^%$", spdlog::pattern_time_type::local, "\n") == "\n");
}

TEST_CASE("color range test3", "[pattern_formatter]")
{
    auto formatter = std::make_shared<spdlog::pattern_formatter>("%^***%$");
    std::string logger_name = "test";
    spdlog::details::log_msg msg(&logger_name, spdlog::level::info, "ignored");
    fmt::memory_buffer formatted;
    formatter->format(msg, formatted);
    REQUIRE(msg.color_range_start == 0);
    REQUIRE(msg.color_range_end == 3);
}

TEST_CASE("color range test4", "[pattern_formatter]")
{
    auto formatter = std::make_shared<spdlog::pattern_formatter>("XX%^YYY%$", spdlog::pattern_time_type::local, "\n");
    std::string logger_name = "test";
    spdlog::details::log_msg msg(&logger_name, spdlog::level::info, "ignored");

    fmt::memory_buffer formatted;
    formatter->format(msg, formatted);
    REQUIRE(msg.color_range_start == 2);
    REQUIRE(msg.color_range_end == 5);
    REQUIRE(log_to_str("ignored", "XX%^YYY%$", spdlog::pattern_time_type::local, "\n") == "XXYYY\n");
}

TEST_CASE("color range test5", "[pattern_formatter]")
{
    auto formatter = std::make_shared<spdlog::pattern_formatter>("**%^");
    std::string logger_name = "test";
    spdlog::details::log_msg msg(&logger_name, spdlog::level::info, "ignored");
    fmt::memory_buffer formatted;
    formatter->format(msg, formatted);
    REQUIRE(msg.color_range_start == 2);
    REQUIRE(msg.color_range_end == 0);
}

TEST_CASE("color range test6", "[pattern_formatter]")
{
    auto formatter = std::make_shared<spdlog::pattern_formatter>("**%$");
    std::string logger_name = "test";
    spdlog::details::log_msg msg(&logger_name, spdlog::level::info, "ignored");
    fmt::memory_buffer formatted;
    formatter->format(msg, formatted);
    REQUIRE(msg.color_range_start == 0);
    REQUIRE(msg.color_range_end == 2);
}
