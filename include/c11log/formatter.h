#pragma once
#include "details/log_msg.h"
namespace c11log
{

class formatter
{
public:
    virtual formatter() {}
    virtual void format(details::log_msg& msg) = 0;
};
}
