#pragma once

#include <string>
#include<cstddef>
#include "../include/spdlog/spdlog.h"


#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
#define  count_lines_t			wcount_lines
#define	file_contents_t			wfile_contents
#else
#define  count_lines_t			count_lines
#define	file_contents_t			file_contents
#endif

std::size_t estimated_file_size(unsigned long nChars);

std::size_t count_lines(const spdlog::filename_t & filename);

void prepare_logdir();

std::wstring wfile_contents(const spdlog::filename_t & filename);
std::string file_contents(const spdlog::filename_t & filename);

std::size_t wcount_lines(const spdlog::filename_t& filename);
std::size_t count_lines(const spdlog::filename_t & filename);

std::size_t get_filesize(const spdlog::filename_t & filename);

