/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#define _CRT_SECURE_NO_WARNINGS
#include "includes.h"
#include <windows.h>
#include <chrono>
#include <vector>

#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
#define  count_lines_t	wcount_lines
#else
#define  count_lines_t	count_lines
#endif

using namespace spdlog;

spdlog::SPDLOGLB_CREATELOGGER_PARAM CLF = { _SLT("%v"), spdlog::level::trace ,0x0FFFFFFFF };






//TEST_CASE("time_format_file_logger", "[simple_logger]]")
//{
//
//
//	spdlog::SPDLOGLB_CREATELOGGER_PARAM lp = { _SLT("%H:%M:%S:(%e|%f|%F) %v"), spdlog::level::trace ,0x0FFFFFFFF };
//
//
//	LARGE_INTEGER qpf;
//	LARGE_INTEGER CurCnt;
//	QueryPerformanceFrequency(&qpf);
//	QueryPerformanceCounter(&CurCnt);
//
//	initialize_library(false, 0, 0, 0);
//	prepare_logdir();
//	spdlb_test::filename_t filename = _SFT("logs/simple_log.txt");
//	auto logger = spdlog::SpdLog_CreateSimpleFileLogger("logger", filename.c_str(), false, true, &lp);
//
//
//
//	std::chrono::seconds d1(42);
//	std::chrono::milliseconds d2(98000);
//	std::chrono::milliseconds d3 = d1 + d2;
//	std::chrono::nanoseconds d1Ns = std::chrono::nanoseconds(d1);
//	std::chrono::nanoseconds d2Ns = std::chrono::nanoseconds(d2);
//	std::chrono::nanoseconds d3Ns = std::chrono::nanoseconds(d3);
//	std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
//	time_t tt = std::chrono::system_clock::to_time_t(tp);
//	std::chrono::steady_clock::time_point tp2 = std::chrono::steady_clock::now();
//	std::chrono::high_resolution_clock::time_point tp3 = std::chrono::high_resolution_clock::now();
//	time_t tte1 = tp.time_since_epoch().count();
//	time_t tte2 = tp2.time_since_epoch().count();
//
//	std::chrono::system_clock::time_point tpTest1 = std::chrono::system_clock::now();
//	std::chrono::system_clock::time_point tpTest2 = std::chrono::system_clock::now();
//	std::chrono::system_clock::time_point tpTest3 = tpTest2 - tpTest1.time_since_epoch();
//	std::chrono::system_clock::time_point tpTest4 = tpTest1 + tpTest2.time_since_epoch();
//
//	auto epoch = std::chrono::time_point<std::chrono::system_clock>();
//	epoch = std::chrono::system_clock::now();
//	auto since_epoch = std::chrono::milliseconds(100000);
//	auto timestamp = epoch + since_epoch;
//
//	std::tm tm;
//	int64_t  i64test = std::chrono::system_clock::to_time_t(epoch);
//	errno_t err = localtime_s(&tm, &i64test);
//	i64test = std::chrono::system_clock::to_time_t(timestamp);
//	err = localtime_s(&tm, &i64test);
//
//	std::chrono::system_clock::time_point tpScNow = std::chrono::system_clock::now();
//	i64test = std::chrono::system_clock::to_time_t(tpScNow);
//	err = localtime_s(&tm, &i64test);
//
//	std::chrono::system_clock::time_point duration = std::chrono::system_clock::now();
//	tm = localtime(std::chrono::system_clock::to_time_t(duration));
//
//	i64test = std::mktime(&tm);
//	i64test += 60;
//	duration = std::chrono::system_clock::from_time_t(i64test);
//	tm = localtime(std::chrono::system_clock::to_time_t(duration));
//
////	std::chrono::system_clock::from_time_t()
////	using SC = std::chrono;
//	std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> tpn = std::chrono::steady_clock::now();
//	//std::chrono::time_point<std::chrono::steady_clock, std::chrono::microseconds> tpn1 = std::chrono::steady_clock::now();
//	//std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> tpn2 = std::chrono::steady_clock::now();
//	//std::chrono::time_point<std::chrono::steady_clock, std::chrono::minutes> tpn3 = std::chrono::steady_clock::now();
//
//	std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> tpsn = std::chrono::system_clock::now();
//	//std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds> tpsn1(tpsn);// = std::chrono::system_clock::now();
//	//std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tpsn2 = std::chrono::system_clock::now();
//	//std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes> tpsn3 = std::chrono::system_clock::now();
//
////	time_t tt2 = std::chrono::steady_clock:: to_time_t(tp2);
//
//	d1Ns = tpn.time_since_epoch();
//	d2Ns = tpsn.time_since_epoch();
//
//
////	std::string stTst = NanoSecondsToString(33128234947);
////	std::string stpn = NanoSecondsToString(d1Ns.count());
////	std::string stpsn = NanoSecondsToString(d2Ns.count());
//
////	std::time_t &time_tt;
//
////	auto tm_time = localtime_s(std::chrono::system_clock::to_time_t(msg.time));
////	localtime_s(&tm, &t64);
//
//	long long psd = std::chrono::system_clock::period().den;
//	long long psn = std::chrono::system_clock::period().num;
//	long long pxd = std::chrono::steady_clock::period().den;
//	long long pxn = std::chrono::steady_clock::period().num;
//
//	int64_t itpna = tpn.time_since_epoch().count();
//	int64_t itpsna = tpsn.time_since_epoch().count();
//
//
//	Sleep(100);
//
//	std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> tpnb = std::chrono::steady_clock::now();
//	std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> tpsnb = std::chrono::system_clock::now();
//
//	int64_t itpnb = tpnb.time_since_epoch().count();
//	int64_t itpsnb = tpsnb.time_since_epoch().count();
//
//	err = localtime_s(&tm, &itpna);
//	err = localtime_s(&tm, &itpnb);
//	err = localtime_s(&tm, &itpsna);
//	err = localtime_s(&tm, &itpsnb);
//
//	std::chrono::system_clock::time_point tptest = std::chrono::system_clock::now();
//	i64test = std::chrono::system_clock::to_time_t(tptest);
//	err = localtime_s(&tm, &i64test);
//	std::chrono::system_clock::duration testDur = tptest.time_since_epoch();
//	int64_t testDurCnt = testDur.count();
//
//	FILETIME ft;
//	SYSTEMTIME st;
//	GetSystemTimeAsFileTime(&ft);
//	FileTimeToSystemTime(&ft, &st);
//
//	// filetime-Einheit ist 100 ns
//	LARGE_INTEGER* lpLi = (LARGE_INTEGER*)&ft;
//	lpLi->QuadPart += 30000;					// 3 ms -> 3.000 Ys -> 30.000 FT-Units (100 ns) 
//	FileTimeToSystemTime(&ft, &st);
//
//
//	LARGE_INTEGER ulTest;
//	ulTest.QuadPart = (int64_t(ft.dwHighDateTime) << 32) + ft.dwLowDateTime;
//	ulTest.HighPart = ft.dwHighDateTime;
//	ulTest.LowPart = ft.dwLowDateTime;
//
//
//	long long ltbnd1 = tpn.time_since_epoch().count();
//	long long ltsnd1 = tpsn.time_since_epoch().count() / 100;
//	long long ltbnd2 = tpnb.time_since_epoch().count();
//	long long ltsnd2 = tpsnb.time_since_epoch().count() / 100;
//
//
//	long long ltbnd = tpnb.time_since_epoch().count() - tpn.time_since_epoch().count();
//	long long ltSnd = tpsnb.time_since_epoch().count() - tpsn.time_since_epoch().count();
//
//	hires_clock_value hrcv;
//	SYSTEMTIME st2;
//
////	hrc_hires_time hst;
//
//	for (unsigned long i = 0; i < 1000000; i++)
//	{
//		hires_clock_value* lpClv = new hires_clock_value();
//		//HiResClock.now(hrcv);
//		//hrcv.to_systemtime(st);
//		//hrcv.to_systemtime_hr(hst);
//		GetSystemTimeAsFileTime(&ft);
//		FileTimeToSystemTime(&ft, &st2);
//
//		////std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
//		////std::chrono::steady_clock::time_point tp2 = std::chrono::steady_clock::now();
//		//std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> tpnb = std::chrono::steady_clock::now();
//		//std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> tpsnb = std::chrono::system_clock::now();
//		//std::string sw = NanoSecondsToString(tpsnb.time_since_epoch().count());
//		//std::string ss = NanoSecondsToString(tpnb.time_since_epoch().count());
//
//		spdlog::SpdLog_LogPrintf(logger, spdlog::level::info, _SLT("LogEntry %i ftl(%x) STC(%02u:%02u:%02u:%03u) HRC(%02u:%02u:%02u:%09u)"),i,ft.dwLowDateTime, st2.wHour,st2.wMinute,st2.wSecond,st2.wMilliseconds, lpClv->tm_hour, lpClv->tm_min, lpClv->tm_sec, lpClv->NanoSeconds);
//		delete lpClv;
//	}
//
//
//	spdlog::SpdLog_LoggerFlush(logger);
//
//	free_library();
//}


TEST_CASE("async_file_logger", "[async_file_logger]]")
{
	initialize_library(true, 1024, 0, 0);
	prepare_logdir();
	CFileName filename(_SFT("logs/simple_log.txt"));
	auto logger = spdlog::SpdLog_CreateSimpleFileLogger("logger", filename.c_str(), false, true, &CLF);


	spdlog::vararg_output_stream vas(logger);
	vas << 1 << VOS_WRITE(spdlog::level::info, _SLT("Test message {}"));
	vas << 2 << VOS_WRITE(spdlog::level::info, _SLT("Test message {}"));
	spdlog::SpdLog_LoggerFlush(logger);
	SpdLog_DeleteLogger(logger);
	free_library();

	REQUIRE(_fmt_err_clb.Error == "");
#if defined(_WIN32)
#if defined(SPDLOG_WCHAR_LOGGING)
	REQUIRE(wfile_contents(filename) == /*spdlog::log_string_t*/(L"Test message 1\r\nTest message 2\r\n"));
	REQUIRE(wcount_lines(filename) == 2);
#else // SPDLOG_WCHAR_LOGGING
	REQUIRE(file_contents(filename) == /*spdlog::log_string_t*/("Test message 1\r\nTest message 2\r\n"));
	REQUIRE(count_lines(filename) == 2);
#endif // SPDLOG_WCHAR_LOGGING
#else // WIN32
	REQUIRE(file_contents(filename) == /*spdlog::log_string_t*/("Test message 1\nTest message 2\n"));
	REQUIRE(count_lines(filename) == 2);
#endif

}


TEST_CASE("simple_file_logger", "[simple_logger]]")
{
	initialize_library(false, 0, 0, 0);
	prepare_logdir();
	CFileName filename(_SFT("logs/simple_log.txt"));
    auto logger = spdlog::SpdLog_CreateSimpleFileLogger("logger", filename.c_str(),false,true,&CLF);


	spdlog::vararg_output_stream vas(logger);
	vas << 1 << VOS_WRITE(spdlog::level::info, _SLT("Test message {}"));
	vas << 2 << VOS_WRITE(spdlog::level::info, _SLT("Test message {}"));
	spdlog::SpdLog_LoggerFlush(logger);
	SpdLog_DeleteLogger(logger);
	free_library();

#if defined(_WIN32)
#if defined(SPDLOG_WCHAR_LOGGING)
	REQUIRE(wfile_contents(filename) == /*spdlog::log_string_t*/(L"Test message 1\r\nTest message 2\r\n"));
	REQUIRE(wcount_lines(filename) == 2);
#else // SPDLOG_WCHAR_LOGGING
	REQUIRE(file_contents(filename) == /*spdlog::log_string_t*/("Test message 1\r\nTest message 2\r\n"));
	REQUIRE(count_lines(filename) == 2);
#endif // SPDLOG_WCHAR_LOGGING
#else // WIN32
	REQUIRE(file_contents(filename) == /*spdlog::log_string_t*/("Test message 1\nTest message 2\n"));
	REQUIRE(count_lines(filename) == 2);
#endif

}

TEST_CASE("rotating_file_logger1", "[rotating_logger]]")
{
	initialize_library(false, 0, 0, 0);
	prepare_logdir();
	spdlb_test::filename_t basename = _SFT("logs/rotating_log");
    auto logger = spdlog::SpdLog_CreateRotatingFileLogger("logger", basename.c_str(), _SFT("txt"), 1024, 0, true,&CLF);
	spdlog::vararg_output_stream vas(logger);

	for (int i = 0; i < 10; ++i)
		vas << i << VOS_WRITE(spdlog::level::info, _SLT("Test message {}"));
	spdlog::SpdLog_LoggerFlush(logger);

    auto filename = basename + _SFT(".txt");
    REQUIRE(count_lines_t(filename) == 10);
    for (int i = 0; i < 1000; i++)
		vas << i << VOS_WRITE(spdlog::level::info, _SLT("Test message {}"));

	spdlog::SpdLog_LoggerFlush(logger);

	free_library();
}


TEST_CASE("rotating_file_logger2", "[rotating_logger]]")
{
	initialize_library(false, 0, 0, 0);
	prepare_logdir();
	spdlb_test::filename_t basename = _SFT("logs/rotating_log");
    auto logger = spdlog::SpdLog_CreateRotatingFileLogger("logger", basename.c_str(),_SFT("txt"), 1024, 1, true,&CLF);
	spdlog::vararg_output_stream vas(logger);

	for (int i = 0; i < 10; ++i)
		vas << i << VOS_WRITE(spdlog::level::info, _SLT("Test message {}"));
	//		spdlog::SpdLog_LogFormat(logger, spdlog::level::info, _SLT("Test message {}"), PAS_ i _PAE );

	spdlog::SpdLog_LoggerFlush(logger);
    auto filename = basename + _SFT(".txt");
    REQUIRE(count_lines_t(filename) == 10);
    for (int i = 0; i < 1000; i++)
		vas << i << VOS_WRITE(spdlog::level::info, _SLT("Test message {}"));

	spdlog::SpdLog_LoggerFlush(logger);
    REQUIRE(get_filesize(filename) <= estimated_file_size(1024));
    auto filename1 = basename + _SFT(".1.txt");
    REQUIRE(get_filesize(filename1) <= estimated_file_size(1024));
	free_library();
}


TEST_CASE("daily_logger", "[daily_logger]]")
{

	initialize_library(false, 0, 0, 0);
	prepare_logdir();
    //calculate filename (time based)
	spdlb_test::filename_t basename = _SFT("logs/daily_log");

	std::time_t now_t = time(nullptr);
	std::tm tm;
	localtime_s(&tm, &now_t);


	const unsigned long FIELD_SIZE = 150;
	spdlog::log_char_t Buffer[FIELD_SIZE];
#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
	_snwprintf_s(Buffer, FIELD_SIZE, L"%s_%04d-%02d-%02d_%02d-%02d.txt", basename.c_str(), tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
#else
	_snprintf_s(Buffer, FIELD_SIZE, "%s_%04d-%02d-%02d_%02d-%02d.txt", basename.c_str(), tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
#endif



    auto logger = spdlog::SpdLog_CreateDaylyFileLogger("logger", basename.c_str(), _SFT("txt"), 0, 0, true,&CLF);
	spdlog::vararg_output_stream vas(logger);

    for (int i = 0; i < 10; ++i)
		vas << i << VOS_WRITE(spdlog::level::info, _SLT("Test message {}"));

	spdlog::SpdLog_LoggerFlush(logger);
	auto filename = Buffer;
    REQUIRE(count_lines_t(filename) == 10);
	free_library();
}



// Varying the filename calculator: not possible using the DLL at the current interface state
//TEST_CASE("daily_logger with dateonly calculator", "[daily_logger_dateonly]]")
//{
//    //using sink_type = spdlog::sinks::daily_file_sink<
//    //                  std::mutex,
//    //                  spdlog::sinks::dateonly_daily_file_name_calculator>;
//
//	initialize_library(false, 0, 0, 0);
//	prepare_logdir();
// //   //calculate filename (time based)
//	spdlb_test::filename_t basename = _SFT("logs/daily_dateonly");
// //   std::tm tm = spdlog::details::os::localtime();
//	//spdlog::fmt_memory_writer_t w;
// //   w.write(_SLT("{}_{:04d}-{:02d}-{:02d}.txt"), basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
//
//	std::time_t now_t = time(nullptr);
//	std::tm tm;
//	localtime_s(&tm, &now_t);
//
//	const unsigned long FIELD_SIZE = 150;
//	spdlog::log_char_t Buffer[FIELD_SIZE];
//#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
//	_snwprintf(Buffer, FIELD_SIZE, L"%s_%04d-%02d-%02d.txt", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
//#else
//	_snprintf(Buffer, FIELD_SIZE, "%s_%04d-%02d-%02d-%02d-%02d.txt", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
//#endif
//
//
//    auto logger = spdlog::create<sink_type>("logger", basename, _SFT("txt"), 0, 0, true);
//    for (int i = 0; i < 10; ++i)
//        logger->info(_SLT("Test message {}"), i);
//
//    auto filename = w.str();
//    REQUIRE(count_lines_t(filename) == 10);
//	free_library();
//}

//struct custom_daily_file_name_calculator
//{
//    static spdlb_test::filename_t calc_filename(const spdlb_test::filename_t& basename, const spdlb_test::filename_t& extension)
//    {
//        std::tm tm = spdlog::details::os::localtime();
//		spdlog::fmt_memory_writer_t w;
//        w.write(_SLT("{}{:04d}{:02d}{:02d}.{}"), basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, extension);
//        return w.str();
//    }
//};
//
//TEST_CASE("daily_logger with custom calculator", "[daily_logger_custom]]")
//{
//    using sink_type = spdlog::sinks::daily_file_sink<
//                      std::mutex,
//                      custom_daily_file_name_calculator>;
//
//	initialize_library(false, 0, 0, 0);
//	prepare_logdir();
//    //calculate filename (time based)
//	spdlb_test::filename_t  basename = _SFT("logs/daily_dateonly");
//    std::tm tm = spdlog::details::os::localtime();
//	spdlog::fmt_memory_writer_t w;
//    w.write(_SLT("{}{:04d}{:02d}{:02d}.txt"), basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
//
//    auto logger = spdlog::create<sink_type>("logger", basename, _SFT("txt"), 0, 0, true);
//    for (int i = 0; i < 10; ++i)
//        logger->info(_SLT("Test message {}"), i);
//
//    auto filename = w.str();
//    REQUIRE(count_lines_t(filename) == 10);
//	free_library();
//}

