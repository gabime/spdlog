#include "includes.h"

void prepare_logdir()
{
    spdlog::drop_all();
#ifdef _WIN32
    auto rv = system("del /F /Q logs\\*");
#else
    auto rv = system("rm -f logs/*");
#endif
}


std::string file_contents(const std::string& filename)
{
    std::ifstream ifs(filename);
    if (!ifs)
        throw std::runtime_error("Failed open file ");
    return std::string((std::istreambuf_iterator<char>(ifs)),
                       (std::istreambuf_iterator<char>()));

}

std::size_t count_lines(const std::string& filename)
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

std::size_t get_filesize(const std::string& filename)
{
    std::ifstream ifs(filename, std::ifstream::ate | std::ifstream::binary);
    if (!ifs)
        throw std::runtime_error("Failed open file ");

    return ifs.tellg();
}

