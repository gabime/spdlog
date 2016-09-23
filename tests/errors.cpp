/*
* This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
*/
#include "includes.h"

#include<iostream>


TEST_CASE("default_error_handler", "[errors]]")
{
    prepare_logdir();
	spdlog::filename_t filename = _SFT("logs/simple_log.txt");

    auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("logger", filename, true);
    logger->set_pattern(_SLT("%v"));
    logger->info(_SLT("Test message {} {}"), 1);
    logger->info(_SLT("Test message {}"), 2);
    logger->flush();

#if defined(_WIN32)
#if defined(SPDLOG_WCHAR_LOGGING)
	REQUIRE(wfile_contents(filename) == spdlog::log_string_t(_SLT("Test message 2\r\n")));
#else
	REQUIRE(file_contents(filename) == std::string("Test message 2\r\n"));
#endif
#else
	REQUIRE(file_contents(filename) == std::string("Test message 2\n"));
#endif
	REQUIRE(count_lines_t(filename) == 1);
}


struct custom_ex {};
TEST_CASE("custom_error_handler", "[errors]]")
{
    prepare_logdir();
	spdlog::filename_t filename = _SFT("logs/simple_log.txt");
    auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("logger", filename, true);
	logger->flush_on(spdlog::level::info);
    logger->set_error_handler([=](const std::string& msg)
    {
        throw custom_ex();
    });
    logger->info(_SLT("Good message #1"));
    REQUIRE_THROWS_AS(logger->info(_SLT("Bad format msg {} {}"), _SLT("xxx")), custom_ex);
    logger->info(_SLT("Good message #2"));
    REQUIRE(count_lines_t(filename) == 2);
}

TEST_CASE("async_error_handler", "[errors]]")
{
    prepare_logdir();
	spdlog::log_string_t err_msg(_SLT("log failed with some msg"));
    spdlog::set_async_mode(128);
    spdlog::filename_t filename = _SFT("logs/simple_async_log.txt");
    {
        auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("logger", filename, true);
        logger->set_error_handler([=](const std::string& msg)
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
        logger->info(_SLT("Good message #1"));
        logger->info(_SLT("Bad format msg {} {}"), _SLT("xxx"));
        logger->info(_SLT("Good message #2"));
        spdlog::drop("logger"); //force logger to drain the queue and shutdown
        spdlog::set_sync_mode();
    }
    REQUIRE(count_lines_t(filename) == 2);
#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
	REQUIRE(wfile_contents(_SFT("logs/custom_err.txt")) == err_msg);
#else
	REQUIRE(file_contents("logs/custom_err.txt") == err_msg);
#endif
}
