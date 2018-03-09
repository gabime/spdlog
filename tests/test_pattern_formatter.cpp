#include "includes.h"

// log to str and return it
static std::string log_to_str(const std::string &msg, const std::shared_ptr<spdlog::formatter> &formatter = nullptr)
{
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    spdlog::logger oss_logger("pattern_tester", oss_sink);
    oss_logger.set_level(spdlog::level::info);
    if (formatter)
        oss_logger.set_formatter(formatter);
    oss_logger.info(msg);
    return oss.str();
}

TEST_CASE("custom eol", "[pattern_formatter]")
{
    std::string msg = "Hello custom eol test";
    std::string eol = ";)";
    auto formatter = std::make_shared<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ";)");

    REQUIRE(log_to_str(msg, formatter) == msg + eol);
}

TEST_CASE("empty format", "[pattern_formatter]")
{
    auto formatter = std::make_shared<spdlog::pattern_formatter>("", spdlog::pattern_time_type::local, "");
    REQUIRE(log_to_str("Some message", formatter) == "");
}

TEST_CASE("empty format2", "[pattern_formatter]")
{
    auto formatter = std::make_shared<spdlog::pattern_formatter>("", spdlog::pattern_time_type::local, "\n");
    REQUIRE(log_to_str("Some message", formatter) == "\n");
}

TEST_CASE("level", "[pattern_formatter]")
{
    auto formatter = std::make_shared<spdlog::pattern_formatter>("[%l] %v", spdlog::pattern_time_type::local, "\n");
    REQUIRE(log_to_str("Some message", formatter) == "[info] Some message\n");
}

TEST_CASE("short level", "[pattern_formatter]")
{
    auto formatter = std::make_shared<spdlog::pattern_formatter>("[%L] %v", spdlog::pattern_time_type::local, "\n");
    REQUIRE(log_to_str("Some message", formatter) == "[I] Some message\n");
}

TEST_CASE("name", "[pattern_formatter]")
{
    auto formatter = std::make_shared<spdlog::pattern_formatter>("[%n] %v", spdlog::pattern_time_type::local, "\n");
    REQUIRE(log_to_str("Some message", formatter) == "[pattern_tester] Some message\n");
}

TEST_CASE("date MM/DD/YY ", "[pattern_formatter]")
{
    auto formatter = std::make_shared<spdlog::pattern_formatter>("%D %v", spdlog::pattern_time_type::local, "\n");
    auto now_tm = spdlog::details::os::localtime();
    std::stringstream oss;
    oss << std::setfill('0') << std::setw(2) << now_tm.tm_mon + 1 << "/" << std::setw(2) << now_tm.tm_mday << "/" << std::setw(2)
        << (now_tm.tm_year + 1900) % 1000 << " Some message\n";
    REQUIRE(log_to_str("Some message", formatter) == oss.str());
}
