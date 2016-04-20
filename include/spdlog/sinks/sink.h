//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//


#pragma once

#include <spdlog/details/log_msg.h>

namespace spdlog
{
namespace sinks
{
class sink
{
public:
    virtual ~sink() {}
    virtual void log(const details::log_msg& msg) = 0;
    virtual void flush() = 0;
};
}
}

