/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"

#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
#define  count_lines_t	wcount_lines
#else
#define  count_lines_t	count_lines
#endif

TEST_CASE("simple_file_logger", "[simple_logger]]")
{
    prepare_logdir();
	spdlog::filename_t filename = _SFT("logs/simple_log.txt");

    auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("logger", filename);
    logger->set_pattern(_SLT("%v"));


    logger->info(_SLT("Test message {}"), 1);
    logger->info(_SLT("Test message {}"), 2);
    logger->flush();
#if defined(_WIN32)
#if defined(SPDLOG_WCHAR_LOGGING)
	REQUIRE(wfile_contents(filename) == /*spdlog::log_string_t*/(L"Test message 1\r\nTest message 2\r\n"));
	REQUIRE(wcount_lines(filename) == 2);
#else // SPDLOG_WCHAR_LOGGING
	// \r is removed by the file_contents routine !!!!
	REQUIRE(file_contents(filename) == /*spdlog::log_string_t*/("Test message 1\nTest message 2\n"));
	REQUIRE(count_lines(filename) == 2);
#endif // SPDLOG_WCHAR_LOGGING
#else // WIN32
	REQUIRE(file_contents(filename) == /*spdlog::log_string_t*/("Test message 1\nTest message 2\n"));
	REQUIRE(count_lines(filename) == 2);
#endif

}

TEST_CASE("rotating_file_logger1", "[rotating_logger]]")
{
    prepare_logdir();
	spdlog::filename_t basename = _SFT("logs/rotating_log");
    auto logger = spdlog::rotating_logger_mt("logger", basename, 1024, 0);
	logger->flush_on(spdlog::level::info);
	for (int i = 0; i < 10; ++i)
        logger->info(_SLT("Test message {}"), i);

    auto filename = basename + _SFT(".txt");
    REQUIRE(count_lines_t(filename) == 10);
    for (int i = 0; i < 1000; i++)
        logger->info(_SLT("Test message {}"), i);

}


TEST_CASE("rotating_file_logger2", "[rotating_logger]]")
{
    prepare_logdir();
	spdlog::filename_t basename = _SFT("logs/rotating_log");
    auto logger = spdlog::rotating_logger_mt("logger", basename, 1024, 1);
    for (int i = 0; i < 10; ++i)
        logger->info(_SLT("Test message {}"), i);

    logger->flush();
    auto filename = basename + _SFT(".txt");
    REQUIRE(count_lines_t(filename) == 10);
    for (int i = 0; i < 1000; i++)
        logger->info(_SLT("Test message {}"), i);

    logger->flush();
    REQUIRE(get_filesize(filename) <= estimated_file_size(1024));
    auto filename1 = basename + _SFT(".1.txt");
    REQUIRE(get_filesize(filename1) <= estimated_file_size(1024));
}


TEST_CASE("daily_logger", "[daily_logger]]")
{
    prepare_logdir();
    //calculate filename (time based)
	spdlog::filename_t basename = _SFT("logs/daily_log");
    std::tm tm = spdlog::details::os::localtime();

#ifdef SPDLOG_WCHAR_FILENAMES
	fmt::WMemoryWriter  w;
	w.write(L"{}_{:04d}-{:02d}-{:02d}_{:02d}-{:02d}.txt", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
#else
	fmt::MemoryWriter  w;
	w.write("{}_{:04d}-{:02d}-{:02d}_{:02d}-{:02d}.txt", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
#endif

    auto logger = spdlog::daily_logger_mt("logger", basename, 0, 0);
	logger->flush_on(spdlog::level::info);
	for (int i = 0; i < 10; ++i)
        logger->info(_SLT("Test message {}"), i);

    auto filename = w.str();
    REQUIRE(count_lines_t(filename) == 10);
}


TEST_CASE("daily_logger with dateonly calculator", "[daily_logger_dateonly]]")
{
    using sink_type = spdlog::sinks::daily_file_sink<
                      std::mutex,
                      spdlog::sinks::dateonly_daily_file_name_calculator>;

    prepare_logdir();
    //calculate filename (time based)
	spdlog::filename_t basename = _SFT("logs/daily_dateonly");
    std::tm tm = spdlog::details::os::localtime();

#ifdef SPDLOG_WCHAR_FILENAMES
	fmt::WMemoryWriter  w;
	w.write(L"{}_{:04d}-{:02d}-{:02d}.txt", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	auto logger = spdlog::create<sink_type>("logger", basename, L"txt", 0, 0);

#else
	fmt::MemoryWriter  w;
    w.write("{}_{:04d}-{:02d}-{:02d}.txt", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	auto logger = spdlog::create<sink_type>("logger", basename, "txt", 0, 0);
#endif


    for (int i = 0; i < 10; ++i)
        logger->info(_SLT("Test message {}"), i);

	logger->flush();

    auto filename = w.str();
    REQUIRE(count_lines_t(filename) == 10);
}

struct custom_daily_file_name_calculator
{
    static spdlog::filename_t calc_filename(const spdlog::filename_t& basename, const spdlog::filename_t& extension)
    {
        std::tm tm = spdlog::details::os::localtime();

		#ifdef SPDLOG_WCHAR_FILENAMES
		fmt::WMemoryWriter  w;
		w.write(L"{}{:04d}{:02d}{:02d}.{}", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, extension);
		#else
		fmt::MemoryWriter  w;
        w.write("{}{:04d}{:02d}{:02d}.{}", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, extension);
		#endif
        return w.str();
    }
};

TEST_CASE("daily_logger with custom calculator", "[daily_logger_custom]]")
{
    using sink_type = spdlog::sinks::daily_file_sink<
                      std::mutex,
                      custom_daily_file_name_calculator>;

    prepare_logdir();
    //calculate filename (time based)
	spdlog::filename_t  basename = _SFT("logs/daily_dateonly");
    std::tm tm = spdlog::details::os::localtime();

	#ifdef SPDLOG_WCHAR_FILENAMES
	fmt::WMemoryWriter  w;
	w.write(L"{}{:04d}{:02d}{:02d}.txt", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	auto logger = spdlog::create<sink_type>("logger", basename, L"txt", 0, 0);
	#else
	fmt::MemoryWriter  w;
	w.write("{}{:04d}{:02d}{:02d}.txt", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	auto logger = spdlog::create<sink_type>("logger", basename, "txt", 0, 0);
	#endif

    for (int i = 0; i < 10; ++i)
        logger->info(_SLT("Test message {}"), i);

	logger->flush();

    auto filename = w.str();
    REQUIRE(count_lines_t(filename) == 10);
}

