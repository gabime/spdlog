/*
* This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
*/
#include "includes.h"

#include<iostream>

spdlog::SPDLOGLB_CREATELOGGER_PARAM CLP = { _SLT("%v"), spdlog::level::trace ,0x0FFFFFFFF };



TEST_CASE("default_error_handler", "[errors]]")
{
	initialize_library(false, 0, 0, 0);
	prepare_logdir();
	CFileName filename(_SFT("logs/simple_log.txt"));
	spdlog::SPDLOGLB_CREATELOGGER_PARAM clp = { 0 };
	clp.FormatString = _SLT("%v");
	clp.Level = spdlog::level::trace;
	clp.LogBitMask = 0x0FFFFFFFF;

	spdlog::log_handle_t hLog = spdlog::SpdLog_CreateSimpleFileLogger("logger", filename.c_str(), false,true,&clp);
    //auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("logger", filename, true);
    //logger->set_pattern(_SLT("%v"));
	vararg_list val1; val1.add_int(2);
	spdlog::SpdLog_LogFormat(hLog, spdlog::level::info, _SLT("Test message {} {}"), val1);
	spdlog::SpdLog_LogFormat(hLog, spdlog::level::info, _SLT("Test message {}"), val1);

	spdlog::SpdLog_LoggerFlush(hLog);

	//logger->info(_SLT("Test message {} {}"), 1);
 //   logger->info(_SLT("Test message {}"), 2);
 //   logger->flush();

#if defined(_WIN32)
#if defined(SPDLOG_WCHAR_LOGGING)
	REQUIRE(wfile_contents(filename) == spdlb_test::log_string_t(_SLT("Test message 2\r\n")));
#else
	REQUIRE(file_contents(filename) == std::string("Test message 2\r\n"));
#endif
#else
	REQUIRE(file_contents(filename) == std::string("Test message 2\n"));
#endif
	REQUIRE(count_lines_t(filename) == 1);

	free_library();
}

// conflicts with DLL exception handling
//struct custom_ex {};
//TEST_CASE("custom_error_handler", "[errors]]")
//{
//	initialize_library(false, 0, 0, 0);
//	prepare_logdir();
//	spdlb_test::filename_t filename = _SFT("logs/simple_log.txt");
//    auto hLog = spdlog::SpdLog_CreateSimpleFileLogger("logger", filename.c_str(), false,true,&CLP);
//	spdlog::SpdLog_SetLoggerErrorHandler(hLog,[=](const std::string& msg)
//    {
//        throw custom_ex();
//    });
//
//
//	spdlog::SpdLog_Log(hLog, spdlog::level::info, _SLT("Good message #1"));
//	vararg_list val; val.add_spdlog_string(_SLT("xxx"));
//
//    REQUIRE_THROWS_AS(spdlog::SpdLog_LogFormat(hLog, spdlog::level::info, _SLT("Bad format msg {} {}"),val), custom_ex);
//	spdlog::SpdLog_Log(hLog, spdlog::level::info, _SLT("Good message #2"));
//	spdlog::SpdLog_LoggerFlush(hLog);
//	REQUIRE(count_lines_t(filename) == 2);
//
//	free_library();
//}

TEST_CASE("async_error_handler", "[errors]]")
{
	initialize_library(true, 128, (unsigned long)spdlog::async_overflow_policy::block_retry, 0);
	prepare_logdir();
	spdlb_test::log_string_t err_msg(_SLT("log failed with some msg"));
//    spdlog::set_async_mode(128);
    spdlb_test::filename_t filename = _SFT("logs/simple_async_log.txt");
    {
        auto logger = spdlog::SpdLog_CreateSimpleFileLogger("logger", filename.c_str(), false,true,&CLP);
		spdlog::SpdLog_SetLoggerErrorHandler(logger,[=](const std::string& msg)
        {
#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)

			wchar_t BOM = 0xFEFF;
			std::ofstream outFile("logs/custom_err.txt", std::ios::out | std::ios::binary);
			if (!outFile) throw std::runtime_error("Failed open logs/custom_err.txt");
			outFile.write((char *)&BOM, sizeof(wchar_t));
			outFile.write((char *)err_msg.c_str(), err_msg.length() * sizeof(wchar_t));

			//// Does not work: need to write unicode in order to compare with the message.
			//std::wofstream ofs(L"logs/custom_err.txt");
			//if (!ofs) throw std::runtime_error("Failed open logs/custom_err.txt");
			//ofs << err_msg;
#else
			std::ofstream ofs("logs/custom_err.txt");
			if (!ofs) throw std::runtime_error("Failed open logs/custom_err.txt");
			ofs << err_msg;
#endif
        });
		spdlog::SpdLog_Log(logger, spdlog::level::info, _SLT("Good message #1"));
		vararg_list val; val.add_spdlog_string(_SLT("xxx"));
		spdlog::SpdLog_LogFormat(logger, spdlog::level::info, _SLT("Bad format msg {} {}"), val );
		spdlog::SpdLog_Log(logger, spdlog::level::info, _SLT("Good message #2"));
        spdlog::SpdLog_DeleteLogger(logger); //force logger to drain the queue and shutdown
//        spdlog::set_sync_mode();
    }
    REQUIRE(count_lines_t(filename) == 2);
#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
	REQUIRE(wfile_contents(_SFT("logs/custom_err.txt")) == err_msg);
#else
	REQUIRE(file_contents("logs/custom_err.txt") == err_msg);
#endif
	free_library();
}
