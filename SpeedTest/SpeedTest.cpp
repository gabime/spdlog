// SpeedTest.cpp : Definiert den Einstiegspunkt für die Anwendung.
//

#include "stdafx.h"
#include "SpeedTest.h"
#include "includes.h"
#include "..\spdLogLbTest\hires_clock_impl.h"
#include <set>

#define MAX_LOADSTRING 100

typedef std::vector<unsigned long>		ULONG_VECTOR;

// Globale Variablen:
HINSTANCE hInst;                                // Aktuelle Instanz
WCHAR szTitle[MAX_LOADSTRING];                  // Titelleistentext
WCHAR szWindowClass[MAX_LOADSTRING];            // Klassenname des Hauptfensters

//// Vorwärtsdeklarationen der in diesem Codemodul enthaltenen Funktionen:
//ATOM                MyRegisterClass(HINSTANCE hInstance);
//BOOL                InitInstance(HINSTANCE, int);
//LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


CLoggingPath	g_bl_logpath;
CBralLog		g_monitor_log;
CBralLog		g_profiling_log;

//wchar_t g_spdlog_logfile[_MAX_PATH];
std::wstring g_spdlog_logfile;
std::wstring g_spdlog_logfolder;
std::wstring g_w_ini_filename;


unsigned long brallog_thread_proc(void* lpParam)
{
	DWORD dwThr = GetCurrentThreadId();
	for (unsigned long i = 0; i < 10000; i++)
	{
		g_profiling_log.LogPrintf(0x01, L"Thread(%x) Step(%u)",dwThr,i);
	}
	return 0;
}

unsigned long brallog_thread_proc_simple(void* lpParam)
{
	DWORD dwThr = GetCurrentThreadId();
	for (unsigned long i = 0; i < 10000; i++)
	{
		g_profiling_log.Log(0x01, L"The quick brown fox jumps over the lazy dog");
	}
	return 0;
}


void brallog_run_loggertest(unsigned long nThreads)
{
	prepare_logdir(g_spdlog_logfile);
	g_profiling_log.Init(g_w_ini_filename, g_bl_logpath, L"SpeedTest");

	spdlog::details::hires_clock_value* lpClvStart = new spdlog::details::hires_clock_value();
	int64_t StartValueNs = lpClvStart->hires_clock_now();

	HANDLE hThr[50];
	for (unsigned long i = 0; i < nThreads; i++)
	{
		hThr[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)brallog_thread_proc, NULL, 0, NULL);
	}

	WaitForMultipleObjects(nThreads, hThr, true, INFINITE);

	spdlog::details::hires_clock_value* lpClvEnd = new spdlog::details::hires_clock_value();
	int64_t EndValueNs = lpClvEnd->hires_clock_now();
	unsigned long ulNanoSeconds = EndValueNs - StartValueNs;

	g_monitor_log.LogPrintf(0x01, L"BRALLOG THR(%u) : Start %02u:%02u:%03u:%u Time %u ns", nThreads, lpClvStart->tm_hour, lpClvStart->tm_min, lpClvStart->tm_sec, lpClvStart->NanoSeconds, ulNanoSeconds);

	g_profiling_log.Free();
}

void brallog_run_loggertest_simple(unsigned long nThreads)
{
	prepare_logdir(g_spdlog_logfile);
	g_profiling_log.Init(g_w_ini_filename, g_bl_logpath, L"SpeedTest");

	spdlog::details::hires_clock_value* lpClvStart = new spdlog::details::hires_clock_value();
	int64_t StartValueNs = lpClvStart->hires_clock_now();

	HANDLE hThr[50];
	for (unsigned long i = 0; i < nThreads; i++)
	{
		hThr[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)brallog_thread_proc_simple, NULL, 0, NULL);
	}

	WaitForMultipleObjects(nThreads, hThr, true, INFINITE);

	spdlog::details::hires_clock_value* lpClvEnd = new spdlog::details::hires_clock_value();
	int64_t EndValueNs = lpClvEnd->hires_clock_now();
	unsigned long ulNanoSeconds = EndValueNs - StartValueNs;

	g_monitor_log.LogPrintf(0x01, L"BRALLOG_SIMPLE THR(%u) : Start %02u:%02u:%03u:%u Time %u ns", nThreads, lpClvStart->tm_hour, lpClvStart->tm_min, lpClvStart->tm_sec, lpClvStart->NanoSeconds, ulNanoSeconds);

	g_profiling_log.Free();
}


unsigned long spdlog_thread_proc(spdlog::log_handle_t hLogger)
{
	spdlog::vararg_output_stream OutStream(hLogger);
	//	std::wostringstream oss;

	DWORD dwThr = GetCurrentThreadId();
	for (unsigned long i = 0; i < 10000; i++)
	{
		OutStream << i << dwThr << spdlog::VOS_WRITE(spdlog::level::info, _SLT("Thread({1:x}) Step({0})"));
	}
	return 0;
}


unsigned long spdlog_thread_proc_simple(spdlog::log_handle_t hLogger)
{
//	spdlog::vararg_output_stream OutStream(hLogger);
	//	std::wostringstream oss;

	DWORD dwThr = GetCurrentThreadId();
	for (unsigned long i = 0; i < 10000; i++)
	{
		spdlog::SpdLog_Log(hLogger, spdlog::level::info, L"The Quick brown Fox jumps over the lazy Dog");
//		OutStream << i << dwThr << spdlog::VOS_WRITE(spdlog::level::info, _SLT("Thread({1:x}) Step({0})"));
	}
	return 0;
}



void spdlog_run_loggertest(spdlog::fmt_formatchar_t* format_string, unsigned long QueueSize, unsigned long OverflowPolicy, unsigned long FlushIntervalMs, unsigned long nThreads, bool AsyncMode)
{

#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
	std::wostringstream oss;
#else
	std::ostringstream oss;
#endif


	wchar_t* chMode = L"ASYNC";
	if (AsyncMode == false)
		chMode = L"STD";


	spdlog::SPDLOGLB_CREATELOGGER_PARAM lp = { format_string, spdlog::level::trace ,0x0FFFFFFFF };
	initialize_library(AsyncMode, QueueSize, OverflowPolicy, FlushIntervalMs);
	prepare_logdir(g_spdlog_logfile);
	//	spdlb_test::filename_t filename = _SFT("logs/simple_log.txt");
	auto logger = spdlog::SpdLog_CreateSimpleFileLogger("logger", g_spdlog_logfile.c_str(), false, true, &lp);
	//auto logger = spdlog::SpdLog_CreateOstreamLogger("oss", oss, false, true, &lp);

	spdlog::details::hires_clock_value* lpClvStart = new spdlog::details::hires_clock_value();
	int64_t StartValueNs = lpClvStart->hires_clock_now();

	HANDLE hThr[50];
	for (unsigned long i = 0; i < nThreads; i++)
	{
		hThr[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)spdlog_thread_proc, logger, 0, NULL);
	}

	WaitForMultipleObjects(nThreads, hThr, true, INFINITE);

	spdlog::details::hires_clock_value* lpClvEnd = new spdlog::details::hires_clock_value();
	int64_t EndValueNs = lpClvEnd->hires_clock_now();
	unsigned long ulNanoSeconds = EndValueNs - StartValueNs;

	g_monitor_log.LogPrintf(0x01,L"%s THR(%u) QS(%u) OP(%u) FL(%u): Start %02u:%02u:%03u:%u Time %u ns f(%s)", chMode, nThreads, QueueSize, OverflowPolicy, FlushIntervalMs, lpClvStart->tm_hour, lpClvStart->tm_min, lpClvStart->tm_sec, lpClvStart->NanoSeconds, ulNanoSeconds, format_string);

	spdlog::SpdLog_LoggerFlush(logger);
	spdlog::SpdLog_DeleteLogger(logger);
	free_library();


}

void spdlog_run_loggertest_simple(spdlog::fmt_formatchar_t* format_string, unsigned long QueueSize, unsigned long OverflowPolicy, unsigned long FlushIntervalMs, unsigned long nThreads, bool AsyncMode,ULONG_VECTOR* lpErg)
{

#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
	std::wostringstream oss;
#else
	std::ostringstream oss;
#endif


	wchar_t* chMode = L"ASYNC_SIMPLE";
	if (AsyncMode == false)
		chMode = L"STD_SIMPLE";


	spdlog::SPDLOGLB_CREATELOGGER_PARAM lp = { format_string, spdlog::level::trace ,0x0FFFFFFFF };
	initialize_library(AsyncMode, QueueSize, OverflowPolicy, FlushIntervalMs);
	prepare_logdir(g_spdlog_logfile);
	//	spdlb_test::filename_t filename = _SFT("logs/simple_log.txt");
	auto logger = spdlog::SpdLog_CreateSimpleFileLogger("logger", g_spdlog_logfile.c_str(), false, true, &lp);
	//auto logger = spdlog::SpdLog_CreateOstreamLogger("oss", oss, false, true, &lp);

	spdlog::details::hires_clock_value* lpClvStart = new spdlog::details::hires_clock_value();
	int64_t StartValueNs = lpClvStart->hires_clock_now();

	HANDLE hThr[50];
	for (unsigned long i = 0; i < nThreads; i++)
	{
		hThr[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)spdlog_thread_proc_simple, logger, 0, NULL);
	}

	WaitForMultipleObjects(nThreads, hThr, true, INFINITE);

	spdlog::details::hires_clock_value* lpClvEnd = new spdlog::details::hires_clock_value();
	int64_t EndValueNs = lpClvEnd->hires_clock_now();
	unsigned long ulNanoSeconds = EndValueNs - StartValueNs;

	if (lpErg)
		lpErg->push_back(ulNanoSeconds);

	g_monitor_log.LogPrintf(0x01, L"%s THR(%u) QS(%u) OP(%u) FL(%u): Start %02u:%02u:%03u:%u Time %u ns f(%s)", chMode, nThreads, QueueSize, OverflowPolicy, FlushIntervalMs, lpClvStart->tm_hour, lpClvStart->tm_min, lpClvStart->tm_sec, lpClvStart->NanoSeconds, ulNanoSeconds, format_string);

	spdlog::SpdLog_LoggerFlush(logger);
	spdlog::SpdLog_DeleteLogger(logger);
	free_library();


}


void spdlog_run_loggertest_simple_ods(spdlog::fmt_formatchar_t* format_string, unsigned long QueueSize, unsigned long OverflowPolicy, unsigned long FlushIntervalMs, unsigned long nThreads, bool AsyncMode, ULONG_VECTOR* lpErg)
{


	wchar_t* chMode = L"ODS_ASYNC_SIMPLE";
	if (AsyncMode == false)
		chMode = L"ODS_STD_SIMPLE";


	spdlog::SPDLOGLB_CREATELOGGER_PARAM lp = { format_string, spdlog::level::trace ,0x0FFFFFFFF };
	initialize_library(AsyncMode, QueueSize, OverflowPolicy, FlushIntervalMs);
//	prepare_logdir(g_spdlog_logfile);
	//	spdlb_test::filename_t filename = _SFT("logs/simple_log.txt");
	auto logger = spdlog::SpdLog_CreateMsvcLogger("logger", true, &lp);
	//auto logger = spdlog::SpdLog_CreateOstreamLogger("oss", oss, false, true, &lp);

	spdlog::details::hires_clock_value* lpClvStart = new spdlog::details::hires_clock_value();
	int64_t StartValueNs = lpClvStart->hires_clock_now();

	HANDLE hThr[50];
	for (unsigned long i = 0; i < nThreads; i++)
	{
		hThr[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)spdlog_thread_proc_simple, logger, 0, NULL);
	}

	WaitForMultipleObjects(nThreads, hThr, true, INFINITE);

	spdlog::details::hires_clock_value* lpClvEnd = new spdlog::details::hires_clock_value();
	int64_t EndValueNs = lpClvEnd->hires_clock_now();
	unsigned long ulNanoSeconds = EndValueNs - StartValueNs;

	if (lpErg)
		lpErg->push_back(ulNanoSeconds);

	g_monitor_log.LogPrintf(0x01, L"%s THR(%u) QS(%u) OP(%u) FL(%u): Start %02u:%02u:%03u:%u Time %u ns f(%s)", chMode, nThreads, QueueSize, OverflowPolicy, FlushIntervalMs, lpClvStart->tm_hour, lpClvStart->tm_min, lpClvStart->tm_sec, lpClvStart->NanoSeconds, ulNanoSeconds, format_string);

	spdlog::SpdLog_LoggerFlush(logger);
	spdlog::SpdLog_DeleteLogger(logger);
	free_library();


}


void RunTests(void)
{
	spdlog::fmt_formatchar_t* FullLogFormat = _SLT("%T:%F [%t] %v");
	spdlog::fmt_formatchar_t* SmallLogFormat = _SLT("%v");


	// 1 Thread mit unterschiedlichen Queue-Sizes
	brallog_run_loggertest(1);
	spdlog_run_loggertest(FullLogFormat, 2, 0, 0, 1, false);
	//spdlog_run_loggertest(FullLogFormat, 2, 0, 0, 1, true);
	//spdlog_run_loggertest(FullLogFormat, 4, 0, 0, 1, true);
	//spdlog_run_loggertest(FullLogFormat, 8, 0, 0, 1, true);
	//spdlog_run_loggertest(FullLogFormat, 16, 0, 0, 1, true);
	//spdlog_run_loggertest(FullLogFormat, 32, 0, 0, 1, true);
	//spdlog_run_loggertest(FullLogFormat, 64, 0, 0, 1, true);
	//spdlog_run_loggertest(FullLogFormat, 128, 0, 0, 1, true);
	//spdlog_run_loggertest(FullLogFormat, 256, 0, 0, 1, true);
	spdlog_run_loggertest(FullLogFormat, 512, 0, 0, 1, true);
	spdlog_run_loggertest(FullLogFormat, 1024, 0, 0, 1, true);
	spdlog_run_loggertest(FullLogFormat, 2048, 0, 0, 1, true);

	// 2 Thread mit unterschiedlichen Queue-Sizes
	brallog_run_loggertest(2);
	spdlog_run_loggertest(FullLogFormat, 2, 0, 0, 2, false);
	//spdlog_run_loggertest(FullLogFormat, 2, 0, 0, 2, true);
	//spdlog_run_loggertest(FullLogFormat, 4, 0, 0, 2, true);
	//spdlog_run_loggertest(FullLogFormat, 8, 0, 0, 2, true);
	//spdlog_run_loggertest(FullLogFormat, 16, 0, 0, 2, true);
	//spdlog_run_loggertest(FullLogFormat, 32, 0, 0, 2, true);
	//spdlog_run_loggertest(FullLogFormat, 64, 0, 0, 2, true);
	//spdlog_run_loggertest(FullLogFormat, 128, 0, 0, 2, true);
	//spdlog_run_loggertest(FullLogFormat, 256, 0, 0, 2, true);
	spdlog_run_loggertest(FullLogFormat, 512, 0, 0, 2, true);
	spdlog_run_loggertest(FullLogFormat, 1024, 0, 0, 2, true);
	spdlog_run_loggertest(FullLogFormat, 2048, 0, 0, 2, true);

	// 5 Thread mit unterschiedlichen Queue-Sizes
	brallog_run_loggertest(5);
	spdlog_run_loggertest(FullLogFormat, 2, 0, 0, 5, false);
	//spdlog_run_loggertest(FullLogFormat, 2, 0, 0, 5, true);
	//spdlog_run_loggertest(FullLogFormat, 4, 0, 0, 5, true);
	//spdlog_run_loggertest(FullLogFormat, 8, 0, 0, 5, true);
	//spdlog_run_loggertest(FullLogFormat, 16, 0, 0, 5, true);
	//spdlog_run_loggertest(FullLogFormat, 32, 0, 0, 5, true);
	//spdlog_run_loggertest(FullLogFormat, 64, 0, 0, 5, true);
	//spdlog_run_loggertest(FullLogFormat, 128, 0, 0, 5, true);
	//spdlog_run_loggertest(FullLogFormat, 256, 0, 0, 5, true);
	spdlog_run_loggertest(FullLogFormat, 512, 0, 0, 5, true);
	spdlog_run_loggertest(FullLogFormat, 1024, 0, 0, 5, true);
	spdlog_run_loggertest(FullLogFormat, 2048, 0, 0, 5, true);

	// 10 Thread mit unterschiedlichen Queue-Sizes
	brallog_run_loggertest(10);
	spdlog_run_loggertest(FullLogFormat, 2, 0, 0, 10, false);
	//spdlog_run_loggertest(FullLogFormat, 2, 0, 0, 10, true);
	//spdlog_run_loggertest(FullLogFormat, 4, 0, 0, 10, true);
	//spdlog_run_loggertest(FullLogFormat, 8, 0, 0, 10, true);
	//spdlog_run_loggertest(FullLogFormat, 16, 0, 0, 10, true);
	//spdlog_run_loggertest(FullLogFormat, 32, 0, 0, 10, true);
	//spdlog_run_loggertest(FullLogFormat, 64, 0, 0, 10, true);
	//spdlog_run_loggertest(FullLogFormat, 128, 0, 0, 10, true);
	//spdlog_run_loggertest(FullLogFormat, 256, 0, 0, 10, true);
	spdlog_run_loggertest(FullLogFormat, 512, 0, 0, 10, true);
	spdlog_run_loggertest(FullLogFormat, 1024, 0, 0, 10, true);
	spdlog_run_loggertest(FullLogFormat, 2048, 0, 0, 10, true);

	// Simpler FormatString
	// 1 Thread mit unterschiedlichen Queue-Sizes
	brallog_run_loggertest(1);
	spdlog_run_loggertest(SmallLogFormat, 2, 0, 0, 1, false);
	//spdlog_run_loggertest(SmallLogFormat, 2, 0, 0, 1, true);
	//spdlog_run_loggertest(SmallLogFormat, 4, 0, 0, 1, true);
	//spdlog_run_loggertest(SmallLogFormat, 8, 0, 0, 1, true);
	//spdlog_run_loggertest(SmallLogFormat, 16, 0, 0, 1, true);
	//spdlog_run_loggertest(SmallLogFormat, 32, 0, 0, 1, true);
	//spdlog_run_loggertest(SmallLogFormat, 64, 0, 0, 1, true);
	//spdlog_run_loggertest(SmallLogFormat, 128, 0, 0, 1, true);
	//spdlog_run_loggertest(SmallLogFormat, 256, 0, 0, 1, true);
	spdlog_run_loggertest(SmallLogFormat, 512, 0, 0, 1, true);
	spdlog_run_loggertest(SmallLogFormat, 1024, 0, 0, 1, true);
	spdlog_run_loggertest(SmallLogFormat, 2048, 0, 0, 1, true);

	// 2 Thread mit unterschiedlichen Queue-Sizes
	brallog_run_loggertest(2);
	spdlog_run_loggertest(SmallLogFormat, 2, 0, 0, 2, false);
	spdlog_run_loggertest(SmallLogFormat, 2, 0, 0, 2, true);
	//spdlog_run_loggertest(SmallLogFormat, 4, 0, 0, 2, true);
	//spdlog_run_loggertest(SmallLogFormat, 8, 0, 0, 2, true);
	//spdlog_run_loggertest(SmallLogFormat, 16, 0, 0, 2, true);
	//spdlog_run_loggertest(SmallLogFormat, 32, 0, 0, 2, true);
	//spdlog_run_loggertest(SmallLogFormat, 64, 0, 0, 2, true);
	//spdlog_run_loggertest(SmallLogFormat, 128, 0, 0, 2, true);
	//spdlog_run_loggertest(SmallLogFormat, 256, 0, 0, 2, true);
	spdlog_run_loggertest(SmallLogFormat, 512, 0, 0, 2, true);
	spdlog_run_loggertest(SmallLogFormat, 1024, 0, 0, 2, true);
	spdlog_run_loggertest(SmallLogFormat, 2048, 0, 0, 2, true);

	// 5 Thread mit unterschiedlichen Queue-Sizes
	brallog_run_loggertest(5);
	spdlog_run_loggertest(SmallLogFormat, 2, 0, 0, 5, false);
	//spdlog_run_loggertest(SmallLogFormat, 2, 0, 0, 5, true);
	//spdlog_run_loggertest(SmallLogFormat, 4, 0, 0, 5, true);
	//spdlog_run_loggertest(SmallLogFormat, 8, 0, 0, 5, true);
	//spdlog_run_loggertest(SmallLogFormat, 16, 0, 0, 5, true);
	//spdlog_run_loggertest(SmallLogFormat, 32, 0, 0, 5, true);
	//spdlog_run_loggertest(SmallLogFormat, 64, 0, 0, 5, true);
	//spdlog_run_loggertest(SmallLogFormat, 128, 0, 0, 5, true);
	//spdlog_run_loggertest(SmallLogFormat, 256, 0, 0, 5, true);
	spdlog_run_loggertest(SmallLogFormat, 512, 0, 0, 5, true);
	spdlog_run_loggertest(SmallLogFormat, 1024, 0, 0, 5, true);
	spdlog_run_loggertest(SmallLogFormat, 2048, 0, 0, 5, true);

	// 10 Thread mit unterschiedlichen Queue-Sizes
	brallog_run_loggertest(10);
	spdlog_run_loggertest(SmallLogFormat, 2, 0, 0, 10, false);
	//spdlog_run_loggertest(SmallLogFormat, 2, 0, 0, 10, true);
	//spdlog_run_loggertest(SmallLogFormat, 4, 0, 0, 10, true);
	//spdlog_run_loggertest(SmallLogFormat, 8, 0, 0, 10, true);
	//spdlog_run_loggertest(SmallLogFormat, 16, 0, 0, 10, true);
	//spdlog_run_loggertest(SmallLogFormat, 32, 0, 0, 10, true);
	//spdlog_run_loggertest(SmallLogFormat, 64, 0, 0, 10, true);
	//spdlog_run_loggertest(SmallLogFormat, 128, 0, 0, 10, true);
	//spdlog_run_loggertest(SmallLogFormat, 256, 0, 0, 10, true);
	spdlog_run_loggertest(SmallLogFormat, 512, 0, 0, 10, true);
	spdlog_run_loggertest(SmallLogFormat, 1024, 0, 0, 10, true);
	spdlog_run_loggertest(SmallLogFormat, 2048, 0, 0, 10, true);

//simple_test:
	// Kein Formatstring
	// 1 Thread mit unterschiedlichen Queue-Sizes
	brallog_run_loggertest_simple(1);
	spdlog_run_loggertest_simple(SmallLogFormat, 2, 0, 0, 1, false,NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 2, 0, 0, 1, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 4, 0, 0, 1, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 8, 0, 0, 1, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 16, 0, 0, 1, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 32, 0, 0, 1, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 64, 0, 0, 1, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 128, 0, 0, 1, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 256, 0, 0, 1, true, NULL);
	spdlog_run_loggertest_simple(SmallLogFormat, 512, 0, 0, 1, true, NULL);
	spdlog_run_loggertest_simple(SmallLogFormat, 1024, 0, 0, 1, true, NULL);
	spdlog_run_loggertest_simple(SmallLogFormat, 2048, 0, 0, 1, true, NULL);

	// 2 Thread mit unterschiedlichen Queue-Sizes
	brallog_run_loggertest_simple(2);
	spdlog_run_loggertest_simple(SmallLogFormat, 2, 0, 0, 2, false, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 2, 0, 0, 2, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 4, 0, 0, 2, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 8, 0, 0, 2, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 16, 0, 0, 2, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 32, 0, 0, 2, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 64, 0, 0, 2, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 128, 0, 0, 2, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 256, 0, 0, 2, true, NULL);
	spdlog_run_loggertest_simple(SmallLogFormat, 512, 0, 0, 2, true, NULL);
	spdlog_run_loggertest_simple(SmallLogFormat, 1024, 0, 0, 2, true, NULL);
	spdlog_run_loggertest_simple(SmallLogFormat, 2048, 0, 0, 2, true, NULL);

	// 5 Thread mit unterschiedlichen Queue-Sizes
	brallog_run_loggertest_simple(5);
	spdlog_run_loggertest_simple(SmallLogFormat, 2, 0, 0, 5, false, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 2, 0, 0, 5, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 4, 0, 0, 5, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 8, 0, 0, 5, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 16, 0, 0, 5, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 32, 0, 0, 5, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 64, 0, 0, 5, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 128, 0, 0, 5, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 256, 0, 0, 5, true, NULL);
	spdlog_run_loggertest_simple(SmallLogFormat, 512, 0, 0, 5, true, NULL);
	spdlog_run_loggertest_simple(SmallLogFormat, 1024, 0, 0, 5, true, NULL);
	spdlog_run_loggertest_simple(SmallLogFormat, 2048, 0, 0, 5, true, NULL);

	// 10 Thread mit unterschiedlichen Queue-Sizes
	brallog_run_loggertest_simple(10);
	spdlog_run_loggertest_simple(SmallLogFormat, 2, 0, 0, 10, false, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 2, 0, 0, 10, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 4, 0, 0, 10, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 8, 0, 0, 10, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 16, 0, 0, 10, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 32, 0, 0, 10, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 64, 0, 0, 10, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 128, 0, 0, 10, true, NULL);
	//spdlog_run_loggertest_simple(SmallLogFormat, 256, 0, 0, 10, true, NULL);
	spdlog_run_loggertest_simple(SmallLogFormat, 512, 0, 0, 10, true, NULL);
	spdlog_run_loggertest_simple(SmallLogFormat, 1024, 0, 0, 10, true, NULL);
	spdlog_run_loggertest_simple(SmallLogFormat, 2048, 0, 0, 10, true, NULL);


//async_sync_compare:

	g_monitor_log.LogPrintf(0x01, L"Running SpeedTest: 20 * synchron vs 20 * asynchron at 65535 queue size");


	// 20 * asynchron im Vergleich zu synchron
	ULONG_VECTOR ulv_synchron;
	ULONG_VECTOR ulv_async;
	ULONG_VECTOR ulv_ods_synchron;
	ULONG_VECTOR ulv_ods_async;
	for (DWORD i = 0; i < 20; i++)
	{
		spdlog_run_loggertest_simple(SmallLogFormat, 65536, 0, 0, 10, false, &ulv_synchron);
		spdlog_run_loggertest_simple(SmallLogFormat, 65536, 0, 0, 10, true, &ulv_async);
//		spdlog_run_loggertest_simple_ods(SmallLogFormat, 65536, 0, 0, 10, false, &ulv_ods_synchron);
//		spdlog_run_loggertest_simple_ods(SmallLogFormat, 65536, 0, 0, 10, true, &ulv_ods_async);

	}

	typedef std::multiset<unsigned long>	UL_MULTISET;
	UL_MULTISET ums_sync;
	UL_MULTISET ums_async;
	UL_MULTISET ums_ods_sync;
	UL_MULTISET ums_ods_async;

	ULONG_VECTOR::iterator it;
	for (it = ulv_synchron.begin(); it != ulv_synchron.end(); it++)
	{
		ums_sync.insert(*it);
	}

	for (it = ulv_async.begin(); it != ulv_async.end(); it++)
	{
		ums_async.insert(*it);
	}

	for (it = ulv_ods_synchron.begin(); it != ulv_ods_synchron.end(); it++)
	{
		ums_ods_sync.insert(*it);
	}

	for (it = ulv_ods_async.begin(); it != ulv_ods_async.end(); it++)
	{
		ums_ods_async.insert(*it);
	}


	g_monitor_log.LogPrintf(0x01, L"=== Ordered synchronous values ===");
	UL_MULTISET::iterator itms;
	double dAverageSync = 0;
	for (itms = ums_sync.begin(); itms != ums_sync.end(); itms++)
	{
		g_monitor_log.LogPrintf(0x01, L"SYNC %10u",*itms);
		dAverageSync += *itms;
	}
	dAverageSync /= ums_sync.size();
	g_monitor_log.LogPrintf(0x01, L"Average: %f",dAverageSync);

	g_monitor_log.LogPrintf(0x01, L"=== Ordered asynchronous values ===");
	double dAverageAsync = 0;
	for (itms = ums_async.begin(); itms != ums_async.end(); itms++)
	{
		g_monitor_log.LogPrintf(0x01, L"ASYNC %10u", *itms);
		dAverageAsync += *itms;
	}
	dAverageAsync /= ums_async.size();
	g_monitor_log.LogPrintf(0x01, L"Average: %f", dAverageAsync);


	//g_monitor_log.LogPrintf(0x01, L"=== Ordered OutputDebugString synchron values ===");
	//double dAverageOdsSync = 0;
	//for (itms = ums_ods_sync.begin(); itms != ums_ods_sync.end(); itms++)
	//{
	//	g_monitor_log.LogPrintf(0x01, L"SYNC %10u", *itms);
	//	dAverageOdsSync += *itms;
	//}
	//dAverageOdsSync /= ums_ods_sync.size();
	//g_monitor_log.LogPrintf(0x01, L"Average: %f", dAverageOdsSync);

	//g_monitor_log.LogPrintf(0x01, L"=== Ordered OutputDebugString asynchronous values ===");
	//double dAverageOdsAsync = 0;
	//for (itms = ums_ods_async.begin(); itms != ums_ods_async.end(); itms++)
	//{
	//	g_monitor_log.LogPrintf(0x01, L"ASYNC %10u", *itms);
	//	dAverageOdsAsync += *itms;
	//}
	//dAverageOdsAsync /= ums_ods_async.size();
	//g_monitor_log.LogPrintf(0x01, L"Average: %f", dAverageOdsAsync);



}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


	CLoggingPath clp_monitor;

	WSTRING_AP Path(_MAX_PATH);
	WSTRING_AP Drive(_MAX_DRIVE);
	WSTRING_AP Dir(_MAX_DIR);
	WSTRING_AP Fname(_MAX_FNAME);
	WSTRING_AP IniFileName(_MAX_PATH);

	GetModuleFileNameW(hInstance, Path, _MAX_PATH);
	_wsplitpath(Path, Drive, Dir, Fname, NULL);
	clp_monitor.Dir = Dir;
	clp_monitor.Drive = Drive;
	clp_monitor.FileExt = L"txt";
	clp_monitor.FileName = L"results";
	_wmakepath(IniFileName, Drive, Dir, Fname, L"ini");
	g_monitor_log.Init(IniFileName.s, clp_monitor, Fname.s);
	g_w_ini_filename = IniFileName.s;

	g_bl_logpath.Dir = Dir;
	g_bl_logpath.Dir.append(L"logs\\");
	g_bl_logpath.Drive = Drive;
	g_bl_logpath.FileExt = L"log";
	g_bl_logpath.FileName = L"simple";

	_wmakepath(IniFileName, Drive, g_bl_logpath.Dir.c_str(), L"simple", L"log");
	g_spdlog_logfile = IniFileName;
	_wmakepath(IniFileName, Drive, g_bl_logpath.Dir.c_str(), NULL, NULL);
	g_spdlog_logfolder = IniFileName;

	RunTests();

    //// Globale Zeichenfolgen initialisieren
    //LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    //LoadStringW(hInstance, IDC_SPEEDTEST, szWindowClass, MAX_LOADSTRING);
    //MyRegisterClass(hInstance);

    //// Anwendungsinitialisierung ausführen:
    //if (!InitInstance (hInstance, nCmdShow))
    //{
    //    return FALSE;
    //}

    //HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SPEEDTEST));

    //MSG msg;

    //// Hauptnachrichtenschleife:
    //while (GetMessage(&msg, nullptr, 0, 0))
    //{
    //    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
    //    {
    //        TranslateMessage(&msg);
    //        DispatchMessage(&msg);
    //    }
    //}

    //return (int) msg.wParam;
}



////
////  FUNKTION: MyRegisterClass()
////
////  ZWECK: Registriert die Fensterklasse.
////
//ATOM MyRegisterClass(HINSTANCE hInstance)
//{
//    WNDCLASSEXW wcex;
//
//    wcex.cbSize = sizeof(WNDCLASSEX);
//
//    wcex.style          = CS_HREDRAW | CS_VREDRAW;
//    wcex.lpfnWndProc    = WndProc;
//    wcex.cbClsExtra     = 0;
//    wcex.cbWndExtra     = 0;
//    wcex.hInstance      = hInstance;
//    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SPEEDTEST));
//    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
//    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
//    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SPEEDTEST);
//    wcex.lpszClassName  = szWindowClass;
//    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
//
//    return RegisterClassExW(&wcex);
//}
//
////
////   FUNKTION: InitInstance(HINSTANCE, int)
////
////   ZWECK: Speichert das Instanzenhandle und erstellt das Hauptfenster.
////
////   KOMMENTARE:
////
////        In dieser Funktion wird das Instanzenhandle in einer globalen Variablen gespeichert, und das
////        Hauptprogrammfenster wird erstellt und angezeigt.
////
//BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
//{
//   hInst = hInstance; // Instanzenhandle in der globalen Variablen speichern
//
//   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
//      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
//
//   if (!hWnd)
//   {
//      return FALSE;
//   }
//
//   ShowWindow(hWnd, nCmdShow);
//   UpdateWindow(hWnd);
//
//   return TRUE;
//}
//
////
////  FUNKTION: WndProc(HWND, UINT, WPARAM, LPARAM)
////
////  ZWECK:  Verarbeitet Meldungen vom Hauptfenster.
////
////  WM_COMMAND  - Verarbeiten des Anwendungsmenüs
////  WM_PAINT    - Darstellen des Hauptfensters
////  WM_DESTROY  - Ausgeben einer Beendenmeldung und zurückkehren
////
////
//LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//    switch (message)
//    {
//    case WM_COMMAND:
//        {
//            int wmId = LOWORD(wParam);
//            // Menüauswahl bearbeiten:
//            switch (wmId)
//            {
//            case IDM_ABOUT:
//                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
//                break;
//            case IDM_EXIT:
//                DestroyWindow(hWnd);
//                break;
//            default:
//                return DefWindowProc(hWnd, message, wParam, lParam);
//            }
//        }
//        break;
//    case WM_PAINT:
//        {
//            PAINTSTRUCT ps;
//            HDC hdc = BeginPaint(hWnd, &ps);
//            //TODO: Zeichencode, der hdc verwendet, hier einfügen...
//            EndPaint(hWnd, &ps);
//        }
//        break;
//    case WM_DESTROY:
//        PostQuitMessage(0);
//        break;
//    default:
//        return DefWindowProc(hWnd, message, wParam, lParam);
//    }
//    return 0;
//}
//
//// Meldungshandler für Infofeld.
//INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
//{
//    UNREFERENCED_PARAMETER(lParam);
//    switch (message)
//    {
//    case WM_INITDIALOG:
//        return (INT_PTR)TRUE;
//
//    case WM_COMMAND:
//        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
//        {
//            EndDialog(hDlg, LOWORD(wParam));
//            return (INT_PTR)TRUE;
//        }
//        break;
//    }
//    return (INT_PTR)FALSE;
//}
