//#include "..\include\spdlog\cstyle_interface.h"
#include "includes.h"



TEST_CASE("LogIntoTwoSimpleFileSinks", "[LogIntoTwoSimpleFileSinks]")
{

	initialize_library(false, 0, 0, 0);
	prepare_logdir();
	spdlog::log_handle_t Sinks[2];
	Sinks[0] = spdlog::SpdLog_CreateFileSink(_SFT("logs/simple_log.txt"), false, true);
	Sinks[1] = spdlog::SpdLog_CreateFileSink(_SFT("logs/simple_log2.txt"), false, true);
	spdlog::log_handle_t  hLogger = spdlog::SpdLog_CreateLogger(Sinks, 2, "TestLogger",_SFS("%v"), spdlog::level::warn, 0x011);
	spdlog::SpdLog_FreeSink(Sinks[0]);
	spdlog::SpdLog_FreeSink(Sinks[1]);

	spdlog::SpdLog_Log(hLogger, spdlog::level::info, _SLT("TestItLevelInfo"));										// Supress
	spdlog::SpdLog_Log(hLogger, spdlog::level::warn, _SLT("TestItLevelWarn"));										// Log
	spdlog::SpdLog_Log(hLogger, spdlog::level::critical, _SLT("TestItLevelCritical"));								// Log
	spdlog::SpdLog_LogBf(hLogger, spdlog::level::critical, 0x01, _SLT("TestItBf_BitFlag01_ShouldBeLogged"));		// Log
	spdlog::SpdLog_LogBf(hLogger, spdlog::level::critical, 0x010, _SLT("TestItBf_BitFlag10_ShouldBeLogged"));		// Log
	spdlog::SpdLog_LogBf(hLogger, spdlog::level::critical, 0x02, _SLT("TestItBf_BitFlag02_NotLogged"));			// Supress
	spdlog::SpdLog_LogBfo(hLogger,  0x01, _SLT("TestItBfo_BitFlag01_ShouldBeLogged"));								// Log
	spdlog::SpdLog_LogBfo(hLogger, 0x010, _SLT("TestItBfo_BitFlag10_ShouldBeLogged"));								// Log
	spdlog::SpdLog_LogBfo(hLogger, 0x02, _SLT("TestItBfo_BitFlag02_NotLogged"));									// Supress
	spdlog::SpdLog_LoggerFlush(hLogger);
	REQUIRE(count_lines_t(_SFT("logs/simple_log.txt")) == 6);
	REQUIRE(count_lines_t(_SFT("logs/simple_log2.txt")) == 6);

	spdlog::log_char_t CorTxt[] = _SLT("TestItLevelWarn\r\nTestItLevelCritical\r\nTestItBf_BitFlag01_ShouldBeLogged\r\nTestItBf_BitFlag10_ShouldBeLogged\r\nTestItBfo_BitFlag01_ShouldBeLogged\r\nTestItBfo_BitFlag10_ShouldBeLogged\r\n");
	REQUIRE(file_contents_t(_SFT("logs/simple_log.txt")) == CorTxt);
	REQUIRE(file_contents_t(_SFT("logs/simple_log2.txt")) == CorTxt);

	const unsigned long BUF_SIZE = 100;
	char Buf[BUF_SIZE];
	REQUIRE(spdlog::SpdLog_IsValidLoggerHandle(hLogger) == true);
	REQUIRE(spdlog::SpdLog_GetLoggerName(hLogger, Buf,BUF_SIZE) == true);

	spdlog::SpdLog_DeleteLogger(hLogger);

	REQUIRE(spdlog::SpdLog_IsValidLoggerHandle(hLogger) == false);

	// With a invalid logger handle GetLoggerName will dig its grave, as most functions that takes a logger handle!
//	REQUIRE(spdlog::SpdLog_GetLoggerName(hLogger, Buf, BUF_SIZE) == false);

	free_library();
}


//TEST_CASE("CStyle BaseTest", "[CStyle BaseTest]")
//{
//	std::string s1 = "Ein Test";
//	std::string s2 = "Zwei Test";
//	REQUIRE(s1 == s2);
//}


