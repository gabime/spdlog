#include "includes.h"

void prepare_logdir()
{
    spdlog::drop_all();
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


std::wstring wfile_contents(const spdlog::filename_t & filename)
{

	std::ifstream ifs(filename);

	if (!ifs)
		throw std::runtime_error("Failed open file ");

	std::string ss((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));
	// Hack: just copy the data. No idea how to do it correct in the ifstream / wifstream way!
	std::wstring ws;
	DWORD DataSize = ss.size() / sizeof(wchar_t);
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

std::string file_contents(const const spdlog::filename_t & filename)
{
    std::ifstream ifs(filename);
    if (!ifs)
        throw std::runtime_error("Failed open file ");
    return std::string((std::istreambuf_iterator<char>(ifs)),
                       (std::istreambuf_iterator<char>()));

}

std::size_t wcount_lines(const spdlog::filename_t& filename)
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

std::size_t count_lines(const spdlog::filename_t & filename)
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

std::size_t get_filesize(const spdlog::filename_t & filename)
{
    std::ifstream ifs(filename, std::ifstream::ate | std::ifstream::binary);
    if (!ifs)
        throw std::runtime_error("Failed open file ");

    return static_cast<std::size_t>(ifs.tellg());
}
