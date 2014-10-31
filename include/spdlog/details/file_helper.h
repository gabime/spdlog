#pragma once



// Helper class for file sink
// When failing to open a file, retry several times(5) with small delay between the tries(10 ms)
// Flush to file every X writes (or never if X==0)
// Throw fflog_ exception on errors


#include <cstdio>
#include <string>
#include <thread>
#include <chrono>
#include "os.h"




namespace spdlog
{
namespace details
{

class file_helper
{
public:
    const int open_tries = 5;
    const int open_interval = 10;

    explicit file_helper(const std::size_t flush_inverval):
        _fd(nullptr),
        _flush_inverval(flush_inverval),
        _flush_countdown(flush_inverval) {};

    file_helper(const file_helper&) = delete;

    ~file_helper()
    {
        close();
    }


    void open(const std::string& fname)
    {

        close();

        _filename = fname;
        for (int tries = 0; tries < open_tries; ++tries)
        {
            if(!os::fopen_s(&_fd, fname, "wb"))
                return;

            std::this_thread::sleep_for(std::chrono::milliseconds(open_interval));
        }

        throw spdlog_ex("Failed opening file " + fname + " for writing");
    }

    void close()
    {
        if (_fd)
        {
            std::fclose(_fd);
            _fd = nullptr;
        }
    }

    void write(const log_msg& msg)
    {
        auto& buf = msg.formatted.buf();
        size_t size = buf.size();
        if(std::fwrite(buf.data(), sizeof(char), size, _fd) != size)
            throw spdlog_ex("Failed writing to file " + _filename);

        if(--_flush_countdown == 0)
        {
            std::fflush(_fd);
            _flush_countdown = _flush_inverval;
        }
    }

    const std::string& filename() const
    {
        return _filename;
    }

    static bool file_exists(const std::string& name)
    {
        FILE* file;
        if (!os::fopen_s(&file, name.c_str(), "r"))
        {
            fclose(file);
            return true;
        }
        else
        {
            return false;
        }
    }

private:
    FILE* _fd;
    std::string _filename;
    const std::size_t _flush_inverval;
    std::size_t _flush_countdown;

};
}
}

