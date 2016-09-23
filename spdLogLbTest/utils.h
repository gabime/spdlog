#pragma once

#include <string>
#include<cstddef>
#include "includes.h"
//#include "..\spdloglb\spdloglb.h"
//#include "../include/spdlog/spdlog.h"


#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
#define  count_lines_t			wcount_lines
#define	file_contents_t			wfile_contents
#else
#define  count_lines_t			count_lines
#define	file_contents_t			file_contents
#endif

class fmt_err_clb : public spdlog::error_callback
{
public:
	std::string Error;
	virtual void on_error(const char* lp_msg)
	{
		Error = lp_msg;
		WARN(Error);
	}

};

extern fmt_err_clb _fmt_err_clb;


std::size_t estimated_file_size(unsigned long nChars);

std::size_t count_lines(const spdlb_test::filename_t & filename);

spdlog::InitReturn initialize_library(bool AsyncMode, unsigned long QueueSize, unsigned long OverflowPolicy, unsigned long FlushIntervalMs);
// spdlog::InitReturn initialize_library_e(bool AsyncMode, unsigned long QueueSize, unsigned long OverflowPolicy, unsigned long FlushIntervalMs, spdlog::error_callback* lp_err_clb);
void free_library();

void prepare_logdir();

std::wstring wfile_contents(const spdlb_test::filename_t & filename);
std::string file_contents(const spdlb_test::filename_t & filename);

std::size_t wcount_lines(const spdlb_test::filename_t& filename);
std::size_t count_lines(const spdlb_test::filename_t & filename);

std::size_t get_filesize(const spdlb_test::filename_t & filename);


class CFileName
{
private:
	std::basic_string<spdlog::filename_char_t>	Buffer;

public:
	CFileName(const CFileName&) = delete;
	CFileName& operator=(const CFileName&) = delete;

	operator const spdlog::filename_char_t*() { return Buffer.c_str(); }
	operator spdlog::filename_char_t*() { return (spdlog::filename_char_t*)Buffer.c_str(); }

	operator spdlb_test::filename_t&() { return Buffer; }

	const spdlog::filename_char_t* c_str(void) { return Buffer.c_str(); }

	CFileName(spdlog::filename_char_t* fn)
	{
		Buffer = fn;
#if defined(_WIN32)
		// Quick and dirty hack: convert slashes to backslashes
		spdlog::filename_char_t* lps = (spdlog::filename_char_t*)Buffer.data();

		for (unsigned int i = 0; i < Buffer.size(); i++)
		{
			if (lps[i] == _SFT('/'))
				lps[i] = _SFT('\\');
		}
#endif
	}
};

