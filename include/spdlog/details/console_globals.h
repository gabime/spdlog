// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/details/null_mutex.h"
#include <cstdio>
#include <mutex>

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX // prevent windows redefining min/max
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#endif

namespace spdlog {
namespace details {
struct console_stdout
{
    static std::FILE *stream()
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
    static std::FILE *stream()
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
