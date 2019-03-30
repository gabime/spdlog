// Copyright(c) 2015-present Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlite/spdlite_global.h"

spdlite::logger &spdlite::default_logger()
{
	return spdlite::logger::default_logger();
}

// printf
void spdlite::log_printf(spdlite::level lvl, const char *format, va_list args)
{
	default_logger().log_printf(lvl, format, args);
}

void spdlite::trace_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	log_printf(level::trace, format, args);
	va_end(args);
}

void spdlite::debug_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	log_printf(level::debug, format, args);
	va_end(args);
}

void spdlite::info_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	log_printf(level::info, format, args);
	va_end(args);
}

void spdlite::warn_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	log_printf(level::warn, format, args);
	va_end(args);
}

void spdlite::error_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	log_printf(level::err, format, args);
	va_end(args);
}

void spdlite::critical_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	log_printf(level::critical, format, args);
	va_end(args);
}