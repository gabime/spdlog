
#include "includes.h"

template<class T>
std::string conditional_log(const bool flag, const T& what, spdlog::level::level_enum logger_level)
{
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

    spdlog::logger oss_logger("oss", oss_sink);
    oss_logger.set_level(logger_level);
    oss_logger.set_pattern("%v");

    switch (logger_level)
    {
    case spdlog::level::trace:
        oss_logger.trace_if(flag, what);
        break;
    case spdlog::level::debug:
        oss_logger.debug_if(flag, what);
        break;
    case spdlog::level::info:
        oss_logger.info_if(flag, what);
        break;
    case spdlog::level::warn:
        oss_logger.warn_if(flag, what);
        break;
    case spdlog::level::err:
        oss_logger.error_if(flag, what);
        break;
    case spdlog::level::critical:
        oss_logger.critical_if(flag, what);
        break;
    default:
        break;
    }

    return oss.str().substr(0, oss.str().length() - spdlog::details::os::eol_size);
}

template <typename Arg1, typename... Args>
std::string conditional_log_varags(spdlog::level::level_enum logger_level, const bool flag, const char* fmt, const Arg1& arg1, const Args&... args)
{
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

    spdlog::logger oss_logger("oss", oss_sink);
    oss_logger.set_level(logger_level);
    oss_logger.set_pattern("%v");

    switch (logger_level)
    {
    case spdlog::level::trace:
        oss_logger.trace_if(flag, fmt, arg1, args...);
        break;
    case spdlog::level::debug:
        oss_logger.debug_if(flag, fmt, arg1, args...);
        break;
    case spdlog::level::info:
        oss_logger.info_if(flag, fmt, arg1, args...);
        break;
    case spdlog::level::warn:
        oss_logger.warn_if(flag, fmt, arg1, args...);
        break;
    case spdlog::level::err:
        oss_logger.error_if(flag, fmt, arg1, args...);
        break;
    case spdlog::level::critical:
        oss_logger.critical_if(flag, fmt, arg1, args...);
        break;
    default:
        break;
    }

    return oss.str().substr(0, oss.str().length() - spdlog::details::os::eol_size);
}

#ifdef SPDLOG_WCHAR_TO_UTF8_SUPPORT

template <typename Arg1, typename... Args>
std::wstring conditional_log_varags(spdlog::level::level_enum logger_level, const bool flag, const wchar_t* fmt, const Arg1& arg1, const Args&... args)
{
    std::wstringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

    spdlog::logger oss_logger("oss", oss_sink);
    oss_logger.set_level(logger_level);
    oss_logger.set_pattern("%v");

    switch (logger_level)
    {
    case spdlog::level::trace:
        oss_logger.trace_if(flag, fmt, arg1, args...);
        break;
    case spdlog::level::debug:
        oss_logger.debug_if(flag, fmt, arg1, args...);
        break;
    case spdlog::level::info:
        oss_logger.info_if(flag, fmt, arg1, args...);
        break;
    case spdlog::level::warn:
        oss_logger.warn_if(flag, fmt, arg1, args...);
        break;
    case spdlog::level::err:
        oss_logger.error_if(flag, fmt, arg1, args...);
        break;
    case spdlog::level::critical:
        oss_logger.critical_if(flag, fmt, arg1, args...);
        break;
    default:
        break;
    }

    return oss.str().substr(0, oss.str().length() - spdlog::details::os::eol_size);
}

#endif // SPDLOG_WCHAR_TO_UTF8_SUPPORT

TEST_CASE("conditional_trace_simple", "[conditional_trace_simple]")
{
    //const char
    for (auto i = 0; i < 2; i++)
    {
        REQUIRE(conditional_log((i % 2 == 0), "Hello", spdlog::level::trace) == ( i % 2 == 0 ? "Hello" : ""));
        REQUIRE(conditional_log((i % 2 == 0), "Hello", spdlog::level::debug) == (i % 2 == 0 ? "Hello" : ""));
        REQUIRE(conditional_log((i % 2 == 0), "Hello", spdlog::level::info) == (i % 2 == 0 ? "Hello" : ""));
        REQUIRE(conditional_log((i % 2 == 0), "Hello", spdlog::level::warn) == (i % 2 == 0 ? "Hello" : ""));
        REQUIRE(conditional_log((i % 2 == 0), "Hello", spdlog::level::err) == (i % 2 == 0 ? "Hello" : ""));
        REQUIRE(conditional_log((i % 2 == 0), "Hello", spdlog::level::critical) == (i % 2 == 0 ? "Hello" : ""));
    }
}

TEST_CASE("conditional_trace_varargs", "[conditional_trace_varargs]")
{
    //const char
    for (auto i = 0; i < 2; i++)
    {
#if !defined(SPDLOG_FMT_PRINTF)
        REQUIRE(conditional_log_varags(spdlog::level::trace, (i % 2 == 0), "Hello {}", i) == (i % 2 == 0 ? "Hello " + std::to_string(i) : ""));
        REQUIRE(conditional_log_varags(spdlog::level::debug, (i % 2 == 0), "Hello {}", i) == (i % 2 == 0 ? "Hello " + std::to_string(i) : ""));
        REQUIRE(conditional_log_varags(spdlog::level::info, (i % 2 == 0), "Hello {}", i) == (i % 2 == 0 ? "Hello " + std::to_string(i) : ""));
        REQUIRE(conditional_log_varags(spdlog::level::warn, (i % 2 == 0), "Hello {}", i) == (i % 2 == 0 ? "Hello " + std::to_string(i) : ""));
        REQUIRE(conditional_log_varags(spdlog::level::err, (i % 2 == 0), "Hello {}", i) == (i % 2 == 0 ? "Hello " + std::to_string(i) : ""));
        REQUIRE(conditional_log_varags(spdlog::level::critical, (i % 2 == 0), "Hello {}", i) == (i % 2 == 0 ? "Hello " + std::to_string(i) : ""));

#ifdef SPDLOG_WCHAR_TO_UTF8_SUPPORT
        REQUIRE(conditional_log_varags(spdlog::level::trace, (i % 2 == 0), L"Hello {}", i) == (i % 2 == 0 ? L"Hello " + std::to_wstring(i) : L""));
        REQUIRE(conditional_log_varags(spdlog::level::debug, (i % 2 == 0), L"Hello {}", i) == (i % 2 == 0 ? L"Hello " + std::to_wstring(i) : L""));
        REQUIRE(conditional_log_varags(spdlog::level::info, (i % 2 == 0), L"Hello {}", i) == (i % 2 == 0 ? L"Hello " + std::to_wstring(i) : L""));
        REQUIRE(conditional_log_varags(spdlog::level::warn, (i % 2 == 0), L"Hello {}", i) == (i % 2 == 0 ? L"Hello " + std::to_wstring(i) : L""));
        REQUIRE(conditional_log_varags(spdlog::level::err, (i % 2 == 0), L"Hello {}", i) == (i % 2 == 0 ? L"Hello " + std::to_wstring(i) : L""));
        REQUIRE(conditional_log_varags(spdlog::level::critical, (i % 2 == 0), L"Hello {}", i) == (i % 2 == 0 ? L"Hello " + std::to_wstring(i) : L""));
#endif // SPDLOG_WCHAR_TO_UTF8_SUPPORT

#else
        REQUIRE(conditional_log_varags(spdlog::level::trace, (i % 2 == 0), "Hello %d", i) == (i % 2 == 0 ? "Hello " + std::to_string(i) : ""));
        REQUIRE(conditional_log_varags(spdlog::level::debug, (i % 2 == 0), "Hello %d", i) == (i % 2 == 0 ? "Hello " + std::to_string(i) : ""));
        REQUIRE(conditional_log_varags(spdlog::level::info, (i % 2 == 0), "Hello %d", i) == (i % 2 == 0 ? "Hello " + std::to_string(i) : ""));
        REQUIRE(conditional_log_varags(spdlog::level::warn, (i % 2 == 0), "Hello %d", i) == (i % 2 == 0 ? "Hello " + std::to_string(i) : ""));
        REQUIRE(conditional_log_varags(spdlog::level::err, (i % 2 == 0), "Hello %d", i) == (i % 2 == 0 ? "Hello " + std::to_string(i) : ""));
        REQUIRE(conditional_log_varags(spdlog::level::critical, (i % 2 == 0), "Hello %d", i) == (i % 2 == 0 ? "Hello " + std::to_string(i) : ""));

#ifdef SPDLOG_WCHAR_TO_UTF8_SUPPORT
        REQUIRE(conditional_log_varags(spdlog::level::trace, (i % 2 == 0), L"Hello %d", i) == (i % 2 == 0 ? L"Hello " + std::to_wstring(i) : L""));
        REQUIRE(conditional_log_varags(spdlog::level::debug, (i % 2 == 0), L"Hello %d", i) == (i % 2 == 0 ? L"Hello " + std::to_wstring(i) : L""));
        REQUIRE(conditional_log_varags(spdlog::level::info, (i % 2 == 0), L"Hello %d", i) == (i % 2 == 0 ? L"Hello " + std::to_wstring(i) : L""));
        REQUIRE(conditional_log_varags(spdlog::level::warn, (i % 2 == 0), L"Hello %d", i) == (i % 2 == 0 ? L"Hello " + std::to_wstring(i) : L""));
        REQUIRE(conditional_log_varags(spdlog::level::err, (i % 2 == 0), L"Hello %d", i) == (i % 2 == 0 ? L"Hello " + std::to_wstring(i) : L""));
        REQUIRE(conditional_log_varags(spdlog::level::critical, (i % 2 == 0), L"Hello %d", i) == (i % 2 == 0 ? L"Hello " + std::to_wstring(i) : L""));
#endif // SPDLOG_WCHAR_TO_UTF8_SUPPORT

#endif // !defined(SPDLOG_FMT_PRINTF)
    }
}