#pragma once
// This file is an extraction of "common.h". It is used to make selected types available for the interface
// of "spdloglb.dll", for example the log level enum. The goal: the interface of "spdloglb" should only need
// this file and "tweakme.h" to read the current configuration options.

#include "spdlog\tweakme.h"
#include <functional>
#include <ostream>

namespace spdlog
{


#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
#if _MSC_VER < 1800
typedef wchar_t log_char_t;
typedef wchar_t fmt_formatchar_t;
typedef  std::wostream std_ostream_t;
#else
using log_char_t = wchar_t;
using fmt_formatchar_t = wchar_t;
using std_ostream_t = std::wostream;
#endif


// SFS is for formatstrings, SLT for log strings
// It does not matter, there is no support for the case that
// format strings and log strings are using different character
// encodings. Just historical...
#define _SFS(x)				L ## x
#define _SLT(x)				L ## x

#else
#if _MSC_VER < 1800
typedef char log_char_t;
typedef char fmt_formatchar_t;
typedef std::ostream std_ostream_t;
#else
using log_char_t = char;
using fmt_formatchar_t = char;
using std_ostream_t = std::ostream;
#endif

#define _SFS(x)				x
#define _SLT(x)		x

#endif


//Log level enum
namespace level
{
	typedef enum
	{
		trace = 0,
		debug = 1,
		info = 2,
		warn = 3,
		err = 4,
		critical = 5,
		off = 6
	} level_enum;
} // namespace level


// wchar support for windows file names (SPDLOG_WCHAR_FILENAMES must be defined)
//
#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
#if _MSC_VER < 1800
typedef wchar_t filename_char_t;
#else
using filename_char_t = wchar_t;
#endif
#define _SFT(x)				L ## x
#else
using filename_char_t = char;
#define _SFT(x)		x
#endif

// For exposing resources over extern "C" functions
#if _MSC_VER < 1800
typedef void* log_handle_t;
#else
using log_handle_t = void*;
#endif

//
// Async overflow policy - block by default.
//
#if _MSC_VER < 1800
enum async_overflow_policy
{
	block_retry, // Block / yield / sleep until message can be enqueued
	discard_log_msg // Discard the message it enqueue fails
};
#else
enum class async_overflow_policy
{
	block_retry, // Block / yield / sleep until message can be enqueued
	discard_log_msg // Discard the message it enqueue fails
};
#endif

#if _MSC_VER < 1800
typedef std::function<void(const std::string &err_msg)> log_err_handler;
#else
using log_err_handler = std::function<void(const std::string &err_msg)>;
#endif

} // namespace spdlog
