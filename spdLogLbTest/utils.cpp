//#include "includes.h"

#include "utils.h"

unsigned long InitCount = 0;

fmt_err_clb _fmt_err_clb;


//spdlog::InitReturn initialize_library_e(bool AsyncMode, unsigned long QueueSize, unsigned long OverflowPolicy, unsigned long FlushIntervalMs, spdlog::error_callback* lp_err_clb)
//{
//	if (InitCount > 0)
//		throw std::exception("!!! Library initialized !!!");
//	InitCount++;
//
//	return spdlog::SpdLog_Init(AsyncMode, QueueSize, OverflowPolicy, FlushIntervalMs, lp_err_clb);
//}

spdlog::InitReturn initialize_library(bool AsyncMode, unsigned long QueueSize, unsigned long OverflowPolicy, unsigned long FlushIntervalMs)
{
	if (InitCount > 0)
		throw std::exception("!!! Library initialized !!!");
	InitCount++;
	_fmt_err_clb.Error = "";
	return spdlog::SpdLog_Init(AsyncMode, QueueSize, OverflowPolicy, FlushIntervalMs, &_fmt_err_clb);
}

void free_library()
{
	//if (_fmt_err_clb.Error.size())
	//{
	//	WARN(_fmt_err_clb.Error);
	//}
	InitCount--;
	spdlog::SpdLog_Free();
}

void prepare_logdir()
{
//    spdlog::drop_all();
#ifdef _WIN32
    auto rv = system("del /F /Q logs\\*");
#else
    auto rv = system("rm -f logs/*");
#endif
    (void)rv;
}


std::size_t estimated_file_size(unsigned long nChars)
{
#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
	return (nChars + 1) * sizeof(wchar_t);
#else
	return nChars;
#endif

}


std::wstring wfile_contents(const spdlb_test::filename_t & filename)
{

	std::ifstream ifs(filename);

	if (!ifs)
		throw std::runtime_error("Failed open file ");

	std::string ss((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));
	// Hack: just copy the data. No idea how to do it correct in the ifstream / wifstream way!
	std::wstring ws;
	unsigned long DataSize = ss.size() / sizeof(wchar_t);
	if (DataSize > 1)
	{
		DataSize--;
		const wchar_t* lps = (const wchar_t*)ss.data();
		lps++;
		ws.resize(DataSize, L' ');
		memcpy((void*)ws.data(), lps, DataSize * sizeof(wchar_t));
	}
	return ws;
}

std::string file_contents(const const spdlb_test::filename_t & filename)
{
    std::ifstream ifs(filename);
    if (!ifs)
        throw std::runtime_error("Failed open file ");
    return std::string((std::istreambuf_iterator<char>(ifs)),
                       (std::istreambuf_iterator<char>()));

}

std::size_t wcount_lines(const spdlb_test::filename_t& filename)
{
	std::wifstream ifs(filename);
	if (!ifs)
		throw std::runtime_error("Failed open file ");

	std::wstring line;
	size_t counter = 0;
	while (std::getline(ifs, line))
		counter++;

	// The BOM is treated as line...
	if (counter > 0) counter--;
	return counter;
}

std::size_t count_lines(const spdlb_test::filename_t & filename)
{
    std::ifstream ifs(filename);
    if (!ifs)
        throw std::runtime_error("Failed open file ");

    std::string line;
    size_t counter = 0;
    while(std::getline(ifs, line))
        counter++;
    return counter;
}

std::size_t get_filesize(const spdlb_test::filename_t & filename)
{
    std::ifstream ifs(filename, std::ifstream::ate | std::ifstream::binary);
    if (!ifs)
        throw std::runtime_error("Failed open file ");

    return static_cast<std::size_t>(ifs.tellg());
}
