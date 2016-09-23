#pragma once

#include <cstdio>
#include <fstream>
#include <string>
#include <ostream>
#include <sstream>
#include <chrono>
#include <exception>


//#include "catch.hpp"


#include "spdlog/spdloglb_VarArg.h"
#include "spdlog/spdloglb.h"
#include "BralLog.h"
#include "StrT.h"

namespace spdlb_test
{

#define SPDLOG_EOL "\r\n"
#define WSPDLOG_EOL L"\r\n"



	// Define some common types to avoid to include internal headers of spdlog
#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
	using filename_t = std::wstring;
#else
	using filename_t = std::string;
#endif


#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
	using log_string_t = std::wstring;
	static const wchar_t* eol = WSPDLOG_EOL;
	static int eol_size = (sizeof(WSPDLOG_EOL) / sizeof(wchar_t)) - 1;
#else
	using log_string_t = std::string;
	static const char* eol = SPDLOG_EOL;
	static int eol_size = sizeof(SPDLOG_EOL) - 1;
#endif

#if defined(_WIN32)
	const spdlog::filename_char_t _SLASH = _SFT('\\');
#else
	const spdlog::filename_char_t _SLASH = _SFT('/');
#endif


}


#include "utils.h"

