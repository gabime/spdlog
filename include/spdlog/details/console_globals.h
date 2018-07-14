#pragma once
//
// Copyright(c) 2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "spdlog/details/null_mutex.h"
#include "stdio.h"
#include <mutex>


namespace spdlog {
namespace details {
struct console_stdout
{
    static FILE *stream()
    {
        return stdout;
    }
#ifdef _WIN32
    static HANDLE handle()
    {
        return ::GetStdHandle(STD_OUTPUT_HANDLE);
    }
#endif
};

struct console_stderr
{
    static FILE *stream()
    {
        return stderr;
    }
#ifdef _WIN32
    static HANDLE handle()
    {
        return ::GetStdHandle(STD_ERROR_HANDLE);
    }
#endif
};

struct console_mutex
{
    using mutex_t = std::mutex;
    static mutex_t &mutex()
    {
        static mutex_t s_mutex;
        return s_mutex;
    }
};

struct console_nullmutex
{
    using mutex_t = null_mutex;
    static mutex_t &mutex()
    {
        static mutex_t s_mutex;
        return s_mutex;
    }
};
} // namespace details
} // namespace spdlog
