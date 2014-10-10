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
};
}
}

