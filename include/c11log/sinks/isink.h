#pragma once

#include "../details/log_msg.h"

namespace c11log
{
namespace sinks
{
class isink
{
public:
    virtual void log(const details::log_msg& msg) = 0;
    virtual void enable(bool enabled) = 0;
    virtual bool is_enabled() = 0;
};
}
}

