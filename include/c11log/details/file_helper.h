#pragma once



// Helper class for file sink
// When failing to open a file, retry several times(5) with small delay between the tries(10 ms)
// Flush to file every X writes (or never if X==0)
// Throw fflog_ exception on errors


#include <cstdio>
#include <string>
#include <thread>
#include <chrono>
#include "../common.h"



namespace c11log
{
namespace details
{

class file_helper
{
public:
    static const int open_max_tries = 5;
    static const int sleep_ms_bewteen_tries = 10;

    explicit file_helper(const std::size_t flush_inverval):
        _fd(nullptr),
        _flush_inverval(flush_inverval),
        _flush_countdown(flush_inverval) {};

    file_helper(const file_helper&) = delete;

    ~file_helper()
    {
        if (_fd)
            std::fclose(_fd);
    }


    void open(const std::string& filename)
    {

        if (_fd)
            std::fclose(_fd);

        _filename = filename;
        for (int tries = 0; tries < open_max_tries; ++tries)
        {
            if(!os::fopen_s(&_fd, filename, "wb"))
                return;

            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms_bewteen_tries));
        }

        throw fflog_exception("Failed opening file " + filename + " for writing");
    }

    void close()
    {
        std::fclose(_fd);
        _fd = nullptr;
    }

    void write(const log_msg& msg)
    {
        auto& buf = msg.formatted.buf();
        size_t size = buf.size();
        if(std::fwrite(buf.data(), sizeof(char), size, _fd) != size)
            throw fflog_exception("Failed writing to  file " + _filename);

        if(--_flush_countdown == 0)
        {
            std::fflush(_fd);
            _flush_countdown = _flush_inverval;
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

