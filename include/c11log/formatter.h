#pragma once

#include <string>
#include <chrono>

#include <iomanip>
#include <thread>
#include <cstring>
#include <sstream>

#include "common.h"
#include "details/os.h"
#include "details/log_msg.h"
#include "details/fast_oss.h"


namespace c11log
{

class formatter
{
public:
    virtual void format(details::log_msg& msg) = 0;
};
}
