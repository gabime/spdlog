
#include "includes.h"

template<class T>
spdlog::log_string_t log_info(const T& what, spdlog::level::level_enum logger_level = spdlog::level::info)
{

#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
	std::wostringstream oss;
#else
	std::ostringstream oss;
#endif
	auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

    spdlog::logger oss_logger("oss", oss_sink);
    oss_logger.set_level(logger_level);
    oss_logger.set_pattern(_SLT("%v"));
    oss_logger.info(what);

    return oss.str().substr(0, oss.str().length() - spdlog::details::os::eol_size);
}






TEST_CASE("basic_logging ", "[basic_logging]")
{
    //const char
    REQUIRE(log_info(_SLT("Hello")) == _SLT("Hello"));
    REQUIRE(log_info(_SLT("")) == _SLT(""));

    //std::string
    REQUIRE(log_info(spdlog::log_string_t(_SLT("Hello"))) == _SLT("Hello"));
    REQUIRE(log_info(spdlog::log_string_t()) == spdlog::log_string_t());

    //Numbers
    REQUIRE(log_info(5) == _SLT("5"));
    REQUIRE(log_info(5.6) == _SLT("5.6"));

    //User defined class
    //REQUIRE(log_info(some_logged_class("some_val")) == "some_val");
}


TEST_CASE("log_levels", "[log_levels]")
{
    REQUIRE(log_info(_SLT("Hello"), spdlog::level::err) == _SLT(""));
    REQUIRE(log_info(_SLT("Hello"), spdlog::level::critical) == _SLT(""));
    REQUIRE(log_info(_SLT("Hello"), spdlog::level::info) == _SLT("Hello"));
    REQUIRE(log_info(_SLT("Hello"), spdlog::level::debug) == _SLT("Hello"));
    REQUIRE(log_info(_SLT("Hello"), spdlog::level::trace) == _SLT("Hello"));
}






