#include "stdafx.h"
#include "spdlog/spdloglb.h"

class CBevlLogLevelConverter
{
public:

	spdlog::level::level_enum		_level;
	operator spdlog::level::level_enum() { return _level;}
	CBevlLogLevelConverter(unsigned long log_level)
	{
		_level = spdlog::level::off;
		switch (log_level)
		{
		case spdlog::level::trace:
			_level = spdlog::level::trace;
			break;
		case spdlog::level::debug:
			_level = spdlog::level::debug;
			break;
		case spdlog::level::info:
			_level = spdlog::level::info;
			break;
		case spdlog::level::warn:
			_level = spdlog::level::warn;
			break;
		case spdlog::level::err:
			_level = spdlog::level::err;
			break;
		case spdlog::level::critical:
			_level = spdlog::level::critical;
			break;
		case spdlog::level::off:
			_level = spdlog::level::off;
			break;
		}
	}

};

class CBevlFormatstringConverter : public spdlog::fmt_formatstring_t
{
public:
	CBevlFormatstringConverter(const spdlog::log_char_t * lpStr)
	{
		if (lpStr == nullptr)
			return;

#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
		unsigned long ull = wcslen(lpStr);
#else
		unsigned long ull = strlen(lpStr);
#endif
		// The string types from the DLL interface "BaumEventLog" and the logger itself has to match!
		// (want isolation of types, spdlog parts should hide in the background)
		this->assign(lpStr, ull);

	}
};


class CBevlFnConverter : public spdlog::filename_t
{
public:
	CBevlFnConverter(const spdlog::filename_char_t * lpStr)
	{
		if (lpStr == nullptr)
			return;
#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
		unsigned long ull = wcslen(lpStr);
#else
		unsigned long ull = strlen(lpStr);
#endif
		this->assign(lpStr, ull);
	}
};
