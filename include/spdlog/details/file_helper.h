//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

// Helper class for file sink
// When failing to open a file, retry several times(5) with small delay between the tries(10 ms)
// Throw spdlog_ex exception on errors

#include "spdlog/details/os.h"
#include "spdlog/details/log_msg.h"

#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <cerrno>
#include <algorithm>

namespace spdlog
{
namespace details
{

class base_file_helper
{
public:
    const int open_tries = 5;
    const int open_interval = 10;

    explicit base_file_helper() {}
    virtual ~base_file_helper() {}

    base_file_helper(const base_file_helper&) = delete;
    base_file_helper& operator=(const base_file_helper&) = delete;

    virtual void open(const filename_t& fname, bool truncate) = 0;
    virtual void reopen(bool truncate) = 0;
    virtual void write(const log_msg& msg) = 0;
    virtual void flush() = 0;
    virtual void close() = 0;
    virtual size_t size() = 0;

    const filename_t& filename() const
    {
        return _filename;
    }

    static bool file_exists(const filename_t& name)
    {
        return os::file_exists(name);
    }

protected:
    filename_t _filename;
};

class file_helper : public base_file_helper
{
public:
    explicit file_helper() :
        _fd(nullptr)
    {}

    file_helper(const file_helper&) = delete;
    file_helper& operator=(const file_helper&) = delete;

    virtual ~file_helper() override
    {
        close();
    }

    virtual void open(const filename_t& fname, bool truncate = false) override
    {
        close();
        auto *mode = truncate ? SPDLOG_FILENAME_T("wb") : SPDLOG_FILENAME_T("ab");
        _filename = fname;
        for (int tries = 0; tries < open_tries; ++tries)
        {
            if (!os::fopen_s(&_fd, fname, mode))
                return;

            std::this_thread::sleep_for(std::chrono::milliseconds(open_interval));
        }

        throw spdlog_ex("Failed opening file " + os::filename_to_str(_filename) + " for writing", errno);
    }

    virtual void reopen(bool truncate) override
    {
        if (_filename.empty())
            throw spdlog_ex("Failed re opening file - was not opened before");

        open(_filename, truncate);
    }

    virtual void write(const log_msg& msg) override
    {
        size_t msg_size = msg.formatted.size();
        auto data = msg.formatted.data();
        if (std::fwrite(data, 1, msg_size, _fd) != msg_size)
            throw spdlog_ex("Failed writing to file " + os::filename_to_str(_filename), errno);
    }

    virtual void flush() override
    {
        std::fflush(_fd);
    }

    virtual void close() override
    {
        if (_fd)
        {
            std::fclose(_fd);
            _fd = nullptr;
        }
    }

    virtual size_t size() override
    {
        if (!_fd)
            throw spdlog_ex("Cannot use size() on closed file " + os::filename_to_str(_filename));

        return os::filesize(_fd);
    }

private:
    FILE* _fd;
};

// file_helper_mp - helper class for multi-process version of file sinks
class file_helper_mp : public base_file_helper
{
public:
    explicit file_helper_mp() :
        _fd(-1)
    {}

    file_helper_mp(const file_helper_mp&) = delete;
    file_helper_mp& operator=(const file_helper_mp&) = delete;

    virtual ~file_helper_mp() override
    {
        close();
    }

    virtual void open(const spdlog::filename_t& fname, bool truncate = false) override
    {
        close();

        _filename = fname;
        int mode = SPDLOG_O_RDWR | SPDLOG_O_APPEND | SPDLOG_O_CREATE | SPDLOG_O_BINARY;
        if (truncate)
            mode |= SPDLOG_O_TRUNCATE;

        for (int tries = 0; tries < open_tries; ++tries)
        {
            if (!os::fopen_s(&_fd, fname, mode))
                return;

            std::this_thread::sleep_for(std::chrono::milliseconds(open_interval));
        }

        throw spdlog_ex("Failed opening file " + os::filename_to_str(_filename) + " for writing", errno);
    }

    virtual void reopen(bool truncate) override
    {
        if (_filename.empty())
        {
            throw spdlog_ex("Failed re opening file - was not opened before");
        }

        open(_filename, truncate);
    }

    virtual void write(const spdlog::details::log_msg& msg) override
    {
        os::write_s(_fd, msg.formatted.data(), msg.formatted.size());
    }

    virtual void flush() override
    {
        // We do not flush because writes supposed to be atomic - all
        // data is already in file on disk
    }

    virtual void close() override
    {
        if (_fd >= 0)
        {
            os::close_s(_fd);
            _fd = -1;
        }
    }

    virtual size_t size() override
    {
        if (_fd < 0)
            throw spdlog_ex("Cannot use size() on closed file " + os::filename_to_str(_filename));

        return os::filesize(_fd);
    }

private:
    int _fd;
};
}
}
