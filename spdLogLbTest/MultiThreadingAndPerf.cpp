#include "includes.h"
#include "hires_clock_impl.h"
#include "..\spdloglb\spdloglb_ostream.h"

//#define HIRES_CLOCK_BASE_TEST

// Checking this takes more time than the logging itself....
//#define CHECK_OUTPUT_FILE_CONTENTS

unsigned long ThreadProc(spdlog::log_handle_t hLogger)
{
	spdlog::vararg_output_stream OutStream(hLogger);
//	std::wostringstream oss;

	DWORD dwThr = GetCurrentThreadId();
	for (unsigned long i = 0; i < 10000; i++)
	{
		OutStream << i << dwThr <<  spdlog::VOS_WRITE(spdlog::level::info,_SLT("Thread({1:x}) Step({0})"));
	}
	return 0;
}



TEST_CASE("MultiThreadStdTest", "[MultiThreadStdTest]")
{
	spdlog::SPDLOGLB_CREATELOGGER_PARAM lp = { _SLT("%H:%M:%S:(%F) %v"), spdlog::level::trace ,0x0FFFFFFFF };
	initialize_library(false, 0, 0, 0);
	prepare_logdir();
	spdlb_test::filename_t filename = _SFT("logs/simple_log.txt");
	auto logger = spdlog::SpdLog_CreateSimpleFileLogger("logger", filename.c_str(), false, true, &lp);

	spdlog::details::hires_clock_value* lpClvStart = new spdlog::details::hires_clock_value();
	int64_t StartValueNs = lpClvStart->hires_clock_now();

	HANDLE hThr[10];
	for (unsigned long i = 0; i < 10; i++)
	{
		hThr[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, logger, 0, NULL);
	}
	
	WaitForMultipleObjects(10, hThr, true, INFINITE);

	spdlog::details::hires_clock_value* lpClvEnd = new spdlog::details::hires_clock_value();
	int64_t EndValueNs = lpClvEnd->hires_clock_now();
	unsigned long ulNanoSeconds = EndValueNs - StartValueNs;

	char Buf[300];
	_snprintf_s(Buf, 300, "MultiThreadStdTest: Start %02u:%02u:%03u:%u End %02u:%02u:%03u:%u Time %u ns",lpClvStart->tm_hour,lpClvStart->tm_min, lpClvStart->tm_sec,lpClvStart->NanoSeconds, lpClvEnd->tm_hour, lpClvEnd->tm_min, lpClvEnd->tm_sec, lpClvEnd->NanoSeconds,ulNanoSeconds);
	WARN(Buf);

	spdlog::SpdLog_LoggerFlush(logger);
	spdlog::SpdLog_DeleteLogger(logger);
	free_library();
#ifdef CHECK_OUTPUT_FILE_CONTENTS
	REQUIRE(count_lines_t(filename) == 100000);
#endif

}


void RunAsyncLogger(spdlog::fmt_formatchar_t* format_string, unsigned long QueueSize, unsigned long OverflowPolicy, unsigned long FlushIntervalMs, unsigned long nThreads,bool AsyncMode)
{

#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
	std::wostringstream oss;
#else
	std::ostringstream oss;
#endif


	char* chMode = "ASYNC";
	if (AsyncMode == false)
		chMode = "STD";


	spdlog::SPDLOGLB_CREATELOGGER_PARAM lp = { format_string, spdlog::level::trace ,0x0FFFFFFFF };
	initialize_library(AsyncMode, QueueSize,OverflowPolicy,FlushIntervalMs);
	prepare_logdir();
//	spdlb_test::filename_t filename = _SFT("logs/simple_log.txt");
//	auto logger = spdlog::SpdLog_CreateSimpleFileLogger("logger", filename.c_str(), false, true, &lp);
	auto logger = spdlog::SpdLog_CreateOstreamLogger("oss", oss, false, true, &lp);

	spdlog::details::hires_clock_value* lpClvStart = new spdlog::details::hires_clock_value();
	int64_t StartValueNs = lpClvStart->hires_clock_now();

	HANDLE hThr[50];
	for (unsigned long i = 0; i < nThreads; i++)
	{
		hThr[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, logger, 0, NULL);
	}

	WaitForMultipleObjects(nThreads, hThr, true, INFINITE);

	spdlog::details::hires_clock_value* lpClvEnd = new spdlog::details::hires_clock_value();
	int64_t EndValueNs = lpClvEnd->hires_clock_now();
	unsigned long ulNanoSeconds = EndValueNs - StartValueNs;

	char Buf[300];
	_snprintf_s(Buf, 300, "%s THR(%u) QS(%u) OP(%u) FL(%u): Start %02u:%02u:%03u:%u Time %u ns f(%S)",chMode,nThreads, QueueSize,OverflowPolicy,FlushIntervalMs, lpClvStart->tm_hour, lpClvStart->tm_min, lpClvStart->tm_sec, lpClvStart->NanoSeconds, ulNanoSeconds,format_string);
	WARN(Buf);

	spdlog::SpdLog_LoggerFlush(logger);
	spdlog::SpdLog_DeleteLogger(logger);
	free_library();


}

TEST_CASE("MultiThreadAsyncLogger", "MultiThreadAsyncLogger")
{
	spdlog::fmt_formatchar_t* FullLogFormat = _SLT("%H:%M:%S:(%F) %v");
	spdlog::fmt_formatchar_t* SmallLogFormat = _SLT("%v");

	// 1 Thread mit unterschiedlichen Queue-Sizes
	RunAsyncLogger(FullLogFormat, 2, 0, 0, 1, false);
	RunAsyncLogger(FullLogFormat, 2, 0, 0, 1,true);
	RunAsyncLogger(FullLogFormat, 4, 0, 0, 1,true);
	RunAsyncLogger(FullLogFormat, 8, 0, 0, 1,true);
	RunAsyncLogger(FullLogFormat, 16, 0, 0, 1,true);
	RunAsyncLogger(FullLogFormat, 32, 0, 0, 1,true);
	RunAsyncLogger(FullLogFormat, 64, 0, 0, 1,true);
	RunAsyncLogger(FullLogFormat, 128, 0, 0, 1,true);
	RunAsyncLogger(FullLogFormat, 256, 0, 0, 1,true);
	RunAsyncLogger(FullLogFormat, 512, 0, 0, 1,true);
	RunAsyncLogger(FullLogFormat, 1024, 0, 0, 1,true);
	RunAsyncLogger(FullLogFormat, 2048, 0, 0, 1,true);

	// 2 Thread mit unterschiedlichen Queue-Sizes
	RunAsyncLogger(FullLogFormat, 2, 0, 0, 2, false);
	RunAsyncLogger(FullLogFormat, 2, 0, 0, 2,true);
	RunAsyncLogger(FullLogFormat, 4, 0, 0, 2,true);
	RunAsyncLogger(FullLogFormat, 8, 0, 0, 2,true);
	RunAsyncLogger(FullLogFormat, 16, 0, 0, 2,true);
	RunAsyncLogger(FullLogFormat, 32, 0, 0, 2,true);
	RunAsyncLogger(FullLogFormat, 64, 0, 0, 2,true);
	RunAsyncLogger(FullLogFormat, 128, 0, 0, 2,true);
	RunAsyncLogger(FullLogFormat, 256, 0, 0, 2,true);
	RunAsyncLogger(FullLogFormat, 512, 0, 0, 2,true);
	RunAsyncLogger(FullLogFormat, 1024, 0, 0, 2,true);
	RunAsyncLogger(FullLogFormat, 2048, 0, 0, 2,true);

	// 5 Thread mit unterschiedlichen Queue-Sizes
	RunAsyncLogger(FullLogFormat, 2, 0, 0, 5, false);
	RunAsyncLogger(FullLogFormat, 2, 0, 0, 5,true);
	RunAsyncLogger(FullLogFormat, 4, 0, 0, 5,true);
	RunAsyncLogger(FullLogFormat, 8, 0, 0, 5,true);
	RunAsyncLogger(FullLogFormat, 16, 0, 0, 5,true);
	RunAsyncLogger(FullLogFormat, 32, 0, 0, 5,true);
	RunAsyncLogger(FullLogFormat, 64, 0, 0, 5,true);
	RunAsyncLogger(FullLogFormat, 128, 0, 0, 5,true);
	RunAsyncLogger(FullLogFormat, 256, 0, 0, 5,true);
	RunAsyncLogger(FullLogFormat, 512, 0, 0, 5,true);
	RunAsyncLogger(FullLogFormat, 1024, 0, 0, 5,true);
	RunAsyncLogger(FullLogFormat, 2048, 0, 0, 5,true);

	// 10 Thread mit unterschiedlichen Queue-Sizes
	RunAsyncLogger(FullLogFormat, 2, 0, 0, 10, false);
	RunAsyncLogger(FullLogFormat, 2, 0, 0, 10,true);
	RunAsyncLogger(FullLogFormat, 4, 0, 0, 10,true);
	RunAsyncLogger(FullLogFormat, 8, 0, 0, 10,true);
	RunAsyncLogger(FullLogFormat, 16, 0, 0, 10,true);
	RunAsyncLogger(FullLogFormat, 32, 0, 0, 10,true);
	RunAsyncLogger(FullLogFormat, 64, 0, 0, 10,true);
	RunAsyncLogger(FullLogFormat, 128, 0, 0, 10,true);
	RunAsyncLogger(FullLogFormat, 256, 0, 0, 10,true);
	RunAsyncLogger(FullLogFormat, 512, 0, 0, 10,true);
	RunAsyncLogger(FullLogFormat, 1024, 0, 0, 10,true);
	RunAsyncLogger(FullLogFormat, 2048, 0, 0, 10,true);

	// 1 Thread mit unterschiedlichen Queue-Sizes
	RunAsyncLogger(SmallLogFormat, 2, 0, 0, 1, false);
	RunAsyncLogger(SmallLogFormat, 2, 0, 0, 1,true);
	RunAsyncLogger(SmallLogFormat, 4, 0, 0, 1,true);
	RunAsyncLogger(SmallLogFormat, 8, 0, 0, 1,true);
	RunAsyncLogger(SmallLogFormat, 16, 0, 0, 1,true);
	RunAsyncLogger(SmallLogFormat, 32, 0, 0, 1,true);
	RunAsyncLogger(SmallLogFormat, 64, 0, 0, 1,true);
	RunAsyncLogger(SmallLogFormat, 128, 0, 0, 1,true);
	RunAsyncLogger(SmallLogFormat, 256, 0, 0, 1,true);
	RunAsyncLogger(SmallLogFormat, 512, 0, 0, 1,true);
	RunAsyncLogger(SmallLogFormat, 1024, 0, 0, 1,true);
	RunAsyncLogger(SmallLogFormat, 2048, 0, 0, 1,true);

	// 2 Thread mit unterschiedlichen Queue-Sizes
	RunAsyncLogger(SmallLogFormat, 2, 0, 0, 2, false);
	RunAsyncLogger(SmallLogFormat, 2, 0, 0, 2,true);
	RunAsyncLogger(SmallLogFormat, 4, 0, 0, 2,true);
	RunAsyncLogger(SmallLogFormat, 8, 0, 0, 2,true);
	RunAsyncLogger(SmallLogFormat, 16, 0, 0, 2,true);
	RunAsyncLogger(SmallLogFormat, 32, 0, 0, 2,true);
	RunAsyncLogger(SmallLogFormat, 64, 0, 0, 2,true);
	RunAsyncLogger(SmallLogFormat, 128, 0, 0, 2,true);
	RunAsyncLogger(SmallLogFormat, 256, 0, 0, 2,true);
	RunAsyncLogger(SmallLogFormat, 512, 0, 0, 2,true);
	RunAsyncLogger(SmallLogFormat, 1024, 0, 0, 2,true);
	RunAsyncLogger(SmallLogFormat, 2048, 0, 0, 2,true);

	// 5 Thread mit unterschiedlichen Queue-Sizes
	RunAsyncLogger(SmallLogFormat, 2, 0, 0, 5, false);
	RunAsyncLogger(SmallLogFormat, 2, 0, 0, 5,true);
	RunAsyncLogger(SmallLogFormat, 4, 0, 0, 5,true);
	RunAsyncLogger(SmallLogFormat, 8, 0, 0, 5,true);
	RunAsyncLogger(SmallLogFormat, 16, 0, 0, 5,true);
	RunAsyncLogger(SmallLogFormat, 32, 0, 0, 5,true);
	RunAsyncLogger(SmallLogFormat, 64, 0, 0, 5,true);
	RunAsyncLogger(SmallLogFormat, 128, 0, 0, 5,true);
	RunAsyncLogger(SmallLogFormat, 256, 0, 0, 5,true);
	RunAsyncLogger(SmallLogFormat, 512, 0, 0, 5,true);
	RunAsyncLogger(SmallLogFormat, 1024, 0, 0, 5,true);
	RunAsyncLogger(SmallLogFormat, 2048, 0, 0, 5,true);

	// 10 Thread mit unterschiedlichen Queue-Sizes
	RunAsyncLogger(SmallLogFormat, 2, 0, 0, 10, false);
	RunAsyncLogger(SmallLogFormat, 2, 0, 0, 10,true);
	RunAsyncLogger(SmallLogFormat, 4, 0, 0, 10,true);
	RunAsyncLogger(SmallLogFormat, 8, 0, 0, 10,true);
	RunAsyncLogger(SmallLogFormat, 16, 0, 0, 10,true);
	RunAsyncLogger(SmallLogFormat, 32, 0, 0, 10,true);
	RunAsyncLogger(SmallLogFormat, 64, 0, 0, 10,true);
	RunAsyncLogger(SmallLogFormat, 128, 0, 0, 10,true);
	RunAsyncLogger(SmallLogFormat, 256, 0, 0, 10,true);
	RunAsyncLogger(SmallLogFormat, 512, 0, 0, 10,true);
	RunAsyncLogger(SmallLogFormat, 1024, 0, 0, 10,true);
	RunAsyncLogger(SmallLogFormat, 2048, 0, 0, 10,true);

}


TEST_CASE("MultiThreadAsync128", "[MultiThreadAsync128]")
{
	spdlog::SPDLOGLB_CREATELOGGER_PARAM lp = { _SLT("%H:%M:%S:(%F) %v"), spdlog::level::trace ,0x0FFFFFFFF };
	initialize_library(true, 128, 0, 0);
	prepare_logdir();
	spdlb_test::filename_t filename = _SFT("logs/simple_log.txt");
	auto logger = spdlog::SpdLog_CreateSimpleFileLogger("logger", filename.c_str(), false, true, &lp);

	spdlog::details::hires_clock_value* lpClvStart = new spdlog::details::hires_clock_value();
	int64_t StartValueNs = lpClvStart->hires_clock_now();

	HANDLE hThr[10];
	for (unsigned long i = 0; i < 10; i++)
	{
		hThr[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, logger, 0, NULL);
	}

	WaitForMultipleObjects(10, hThr, true, INFINITE);

	spdlog::details::hires_clock_value* lpClvEnd = new spdlog::details::hires_clock_value();
	int64_t EndValueNs = lpClvEnd->hires_clock_now();
	unsigned long ulNanoSeconds = EndValueNs - StartValueNs;

	char Buf[300];
	_snprintf_s(Buf, 300, "MultiThreadAsync128: Start %02u:%02u:%03u:%u End %02u:%02u:%03u:%u Time %u ns", lpClvStart->tm_hour, lpClvStart->tm_min, lpClvStart->tm_sec, lpClvStart->NanoSeconds, lpClvEnd->tm_hour, lpClvEnd->tm_min, lpClvEnd->tm_sec, lpClvEnd->NanoSeconds, ulNanoSeconds);
	WARN(Buf);

	spdlog::SpdLog_LoggerFlush(logger);
	spdlog::SpdLog_DeleteLogger(logger);
	free_library();
#ifdef CHECK_OUTPUT_FILE_CONTENTS
	REQUIRE(count_lines_t(filename) == 100000);
#endif

}


TEST_CASE("MultiThreadAsync1024", "[MultiThreadAsync1024]")
{
	spdlog::SPDLOGLB_CREATELOGGER_PARAM lp = { _SLT("%H:%M:%S:(%F) %v"), spdlog::level::trace ,0x0FFFFFFFF };
	initialize_library(true, 1024, 0, 0);
	prepare_logdir();
	spdlb_test::filename_t filename = _SFT("logs/simple_log.txt");
	auto logger = spdlog::SpdLog_CreateSimpleFileLogger("logger", filename.c_str(), false, true, &lp);

	spdlog::details::hires_clock_value* lpClvStart = new spdlog::details::hires_clock_value();
	int64_t StartValueNs = lpClvStart->hires_clock_now();

	HANDLE hThr[10];
	for (unsigned long i = 0; i < 10; i++)
	{
		hThr[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, logger, 0, NULL);
	}

	WaitForMultipleObjects(10, hThr, true, INFINITE);

	spdlog::details::hires_clock_value* lpClvEnd = new spdlog::details::hires_clock_value();
	int64_t EndValueNs = lpClvEnd->hires_clock_now();
	unsigned long ulNanoSeconds = EndValueNs - StartValueNs;

	char Buf[300];
	_snprintf_s(Buf, 300, "MultiThreadAsync1024: Start %02u:%02u:%03u:%u End %02u:%02u:%03u:%u Time %u ns", lpClvStart->tm_hour, lpClvStart->tm_min, lpClvStart->tm_sec, lpClvStart->NanoSeconds, lpClvEnd->tm_hour, lpClvEnd->tm_min, lpClvEnd->tm_sec, lpClvEnd->NanoSeconds, ulNanoSeconds);
	WARN(Buf);

	spdlog::SpdLog_LoggerFlush(logger);
	spdlog::SpdLog_DeleteLogger(logger);
	free_library();
#ifdef CHECK_OUTPUT_FILE_CONTENTS
	REQUIRE(count_lines_t(filename) == 100000);
#endif

}


#ifdef HIRES_CLOCK_BASE_TEST
TEST_CASE("HiresClockVsStdClock", "[HiresClockVsStdClock]")
{
	spdlog::SPDLOGLB_CREATELOGGER_PARAM lp = { _SLT("%H:%M:%S:(%F) %v"), spdlog::level::trace ,0x0FFFFFFFF };

	initialize_library(false, 0, 0, 0);
	prepare_logdir();
	spdlb_test::filename_t filename = _SFT("logs/simple_log.txt");
	auto logger = spdlog::SpdLog_CreateSimpleFileLogger("logger", filename.c_str(), false, true, &lp);

	FILETIME ft;
	SYSTEMTIME st2;

	for (unsigned long i = 0; i < 1000; i++)
	{
		spdlog::details::hires_clock_value* lpClv = new spdlog::details::hires_clock_value();
		//	FILETIME ft;

		GetSystemTimeAsFileTime(&ft);
		FileTimeToSystemTime(&ft, &st2);
		spdlog::SpdLog_LogPrintf(logger, spdlog::level::info, _SLT("LogEntry %i ftl(%x) STC(%02u:%02u:%02u:%03u) HRC(%02u:%02u:%02u:%09u)"),i,ft.dwLowDateTime, st2.wHour,st2.wMinute,st2.wSecond,st2.wMilliseconds, lpClv->tm_hour, lpClv->tm_min, lpClv->tm_sec, lpClv->NanoSeconds);
		delete lpClv;
		Sleep(1);
	}
	
	
	spdlog::SpdLog_LoggerFlush(logger);
	spdlog::SpdLog_DeleteLogger(logger);

	free_library();

}
#endif