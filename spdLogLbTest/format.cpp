
#include "includes.h"
#include <excpt.h>
#include "spdlog/spdloglb_ostream.h"
#include "spdlog/spdloglb_VarArg.h"
using namespace spdlog;

template<class T>
spdlb_test::log_string_t log_info(const T& what, spdlog::level::level_enum logger_level = spdlog::level::info)
{

#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
	std::wostringstream oss;
#else
	std::ostringstream oss;
#endif

	spdlog::SPDLOGLB_CREATELOGGER_PARAM clp = { 0 };
	clp.FormatString = _SLT("%v");
	clp.Level = logger_level;
	clp.LogBitMask = 0x0FFFFFFFF;
	auto oss_logger = spdlog::SpdLog_CreateOstreamLogger("oss", oss, false, true, &clp);

	//auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

 //   spdlog::logger oss_logger("oss", oss_sink);
 //   oss_logger.set_level(logger_level);
 //   oss_logger.set_pattern(_SLT("%v"));
    
	spdlog::SpdLog_Log(oss_logger, spdlog::level::info, what);

	
	spdlog::SpdLog_DeleteLogger(oss_logger);

   return oss.str().substr(0, oss.str().length() - spdlb_test::eol_size);
}


TEST_CASE("vararg_format_base ", "[vararg_format_base]")
{
#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
	std::wostringstream oss;
#else
	std::ostringstream oss;
#endif

	initialize_library(false, 0, 0, 0);

	spdlog::SPDLOGLB_CREATELOGGER_PARAM clp = { 0 };
	clp.FormatString = _SLT("%v");
	clp.Level = spdlog::level::info;
	clp.LogBitMask = 0x0FFFFFFFF;
	auto oss_logger = spdlog::SpdLog_CreateOstreamLogger("oss", oss, false, true, &clp);
	spdlog::vararg_output_stream OutStream(oss_logger);

	// int
	int iv = 4;
	OutStream << 3 << iv;
	OutStream << spdlog::VOS_WRITE(spdlog::level::info, _SLT("{} {}"));
//	const wchar_t* lps = oss.str().c_str();
	REQUIRE(oss.str() == _SLT("3 4\r\n"));
	oss.str(L"");

	// More than 20 arguments - transfers types unpacked because they do not fit into one 64-Bit-Value
	OutStream << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10 << 11 << 12 << 13 << 14 << 15 << 16 << 17 << 18 << 19 << 20 << 21 << 22;
	OutStream << spdlog::VOS_WRITE(spdlog::level::info, _SLT("{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}"));
	REQUIRE(oss.str() == _SLT("1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22\r\n"));
	oss.str(L"");

	// Some format options for integer
	int i = 0x0A3;
	OutStream << i << VOS_WRITE(spdlog::level::info, _SLT("{0:b} {0:B} {0:d} {0:o} {0:x} {0:X}"));
	REQUIRE(oss.str() == _SLT("10100011 10100011 163 243 a3 A3\r\n"));
	oss.str(L"");

	OutStream << i << VOS_WRITE(spdlog::level::info, _SLT("{0:#b} {0:#B} {0:d} {0:o} {0:#x} {0:#X}"));
	REQUIRE(oss.str() == _SLT("0b10100011 0B10100011 163 243 0xa3 0XA3\r\n"));
	oss.str(L"");

	i = 4711;

	OutStream << i << VOS_WRITE(spdlog::level::info, _SLT("{0:-^+10d} {0:->+10d} {0:-<+10d}"));
	REQUIRE(oss.str() == _SLT("--+4711--- -----+4711 +4711-----\r\n"));
	oss.str(L"");

	// Strings
	wchar_t* MyWstr = L"WIDE";
	char* MyStr = "narrow";
	OutStream << MyWstr << MyStr << VOS_WRITE(spdlog::level::info, _SLT("{0:s} {1:s}"));
	REQUIRE(oss.str() == _SLT("WIDE narrow\r\n"));
	oss.str(L"");

	// Characters
	MyStr = "abc";
	OutStream << MyStr[0] << MyStr[1] << MyStr[2] << VOS_WRITE(spdlog::level::info, _SLT("{0:c} {1:c} {2:c}"));
	REQUIRE(oss.str() == _SLT("a b c\r\n"));
	oss.str(L"");

	// Pointer type
	void* lpv = NULL;
	OutStream << lpv << VOS_WRITE(spdlog::level::info, _SLT("{0:p}"));
	REQUIRE(oss.str() == _SLT("0x0\r\n"));
	oss.str(L"");

	OutStream << (void*)MyStr << VOS_WRITE(spdlog::level::info, _SLT("{0:p}"));
	oss.str(L"");

	// Floats
	double flv1 = 47.11;
	long double flv2 = 47.11;
	OutStream << flv1 << flv2 << VOS_WRITE(spdlog::level::info, _SLT("{0} {1} {0:a} {1:a} {0:A} {1:A} {0:e} {1:e} {0:E} {1:E} {0:f} {1:f} {0:F} {1:F} {0:g} {1:g} {0:G} {1:G}"));
	REQUIRE(oss.str() == _SLT("47.11 47.11 0x1.78e147ae147aep+5 0x1.78e147ae147aep+5 0X1.78E147AE147AEP+5 0X1.78E147AE147AEP+5 4.711000e+01 4.711000e+01 4.711000E+01 4.711000E+01 47.110000 47.110000 47.110000 47.110000 47.11 47.11 47.11 47.11\r\n"));
	oss.str(L"");


	// Errors

	_fmt_err_clb.Error = "";
	OutStream << MyWstr << VOS_WRITE(spdlog::level::info, _SLT("{0:s} {1:s}"));
	REQUIRE(_fmt_err_clb.Error.size() > 0);

	// This does not cause an error: only the first parameter from two are used...
	_fmt_err_clb.Error = "";
	OutStream << MyWstr << MyStr << VOS_WRITE(spdlog::level::info, _SLT("{0:s} {0:s}"));
	REQUIRE(_fmt_err_clb.Error.size() == 0);

	// wrong format code
	_fmt_err_clb.Error = "";
	OutStream << i  << VOS_WRITE(spdlog::level::info, _SLT("{0:s}"));
	REQUIRE(_fmt_err_clb.Error.size() > 0);


	spdlog::SpdLog_DeleteLogger(oss_logger);
	free_library();
}




TEST_CASE("basic_logging ", "[basic_logging]")
{
	initialize_library(false, 0, 0, 0);

    //const char
    REQUIRE(log_info(_SLT("Hello")) == _SLT("Hello"));
    REQUIRE(log_info(_SLT("")) == _SLT(""));

	// does not make sense, DLL takes just strings, nothing else
    ////std::string
    //REQUIRE(log_info(spdlb_test::log_string_t(_SLT("Hello"))) == _SLT("Hello"));
    //REQUIRE(log_info(spdlb_test::log_string_t()) == spdlb_test::log_string_t());

    ////Numbers
    //REQUIRE(log_info(_SLT("5")) == _SLT("5"));
    //REQUIRE(log_info(_SLT("5.6")) == _SLT("5.6"));

    //User defined class
    //REQUIRE(log_info(some_logged_class("some_val")) == "some_val");
	free_library();
}


TEST_CASE("log_levels", "[log_levels]")
{
	initialize_library(false, 0, 0, 0);
	REQUIRE(log_info(_SLT("Hello"), spdlog::level::err) == _SLT(""));
    REQUIRE(log_info(_SLT("Hello"), spdlog::level::critical) == _SLT(""));
    REQUIRE(log_info(_SLT("Hello"), spdlog::level::info) == _SLT("Hello"));
    REQUIRE(log_info(_SLT("Hello"), spdlog::level::debug) == _SLT("Hello"));
    REQUIRE(log_info(_SLT("Hello"), spdlog::level::trace) == _SLT("Hello"));
	free_library();
}






