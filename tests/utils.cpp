#include "includes.h"

void prepare_logdir()
{
    spdlog::drop_all();
#ifdef _WIN32
    system("if not exist logs mkdir logs");
    system("del /F /Q logs\\*");
#else
    auto rv = system("mkdir -p logs");
    rv = system("rm -f logs/*");
    (void)rv;
#endif
}

std::string file_contents(const std::string &filename)
{
    std::ifstream ifs(filename);
    if (!ifs)
        throw std::runtime_error("Failed open file ");
    return std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
}

std::size_t count_lines(const std::string &filename)
{
    std::ifstream ifs(filename);
    if (!ifs)
        throw std::runtime_error("Failed open file ");

    std::string line;
    size_t counter = 0;
    while (std::getline(ifs, line))
        counter++;
    return counter;
}

std::size_t get_filesize(const std::string &filename)
{
    std::ifstream ifs(filename, std::ifstream::ate | std::ifstream::binary);
    if (!ifs)
        throw std::runtime_error("Failed open file ");

    return static_cast<std::size_t>(ifs.tellg());
}

// source: https://stackoverflow.com/a/2072890/192001
bool ends_with(std::string const &value, std::string const &ending)
{
    if (ending.size() > value.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}
