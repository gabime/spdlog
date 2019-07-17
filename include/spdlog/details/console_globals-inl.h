// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/common.h"
#include "spdlog/details/console_globals.h"

namespace spdlog {
namespace details {

SPDLOG_INLINE console_mutex::mutex_t &console_mutex::mutex()
{
    static mutex_t s_mutex;
    return s_mutex;
}

SPDLOG_INLINE console_nullmutex::mutex_t &console_nullmutex::mutex()
{
    static mutex_t s_mutex;
    return s_mutex;
}

} // namespace details
} // namespace spdlog

