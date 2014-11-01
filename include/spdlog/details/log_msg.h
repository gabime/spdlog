/*************************************************************************/
/* spdlog - an extremely fast and easy to use c++11 logging library.     */
/* Copyright (c) 2014 Gabi Melman.                                       */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

#include "../common.h"
#include "./fast_oss.h"

namespace spdlog
{
namespace details
{
struct log_msg
{
    log_msg() = default;
    log_msg(level::level_enum l):
        logger_name(),
        level(l),
        time(),
        tm_time(),
        raw(),
        formatted() {}

    log_msg(const log_msg& other):
        logger_name(other.logger_name),
        level(other.level),
        time(other.time),
        tm_time(other.tm_time),
        raw(other.raw),
        formatted(other.formatted) {}

    log_msg(log_msg&& other)
    {
        swap(*this, other);
    }

    void swap(log_msg& l, log_msg& r)
    {
        using std::swap;
        swap(l.logger_name, r.logger_name);
        swap(l.level, r.level);
        swap(l.time, r.time);
        swap(l.tm_time, r.tm_time);
        swap(l.raw, r.raw);
        swap(l.formatted, r.formatted);
    }


    log_msg& operator=(log_msg other)
    {
        swap(*this, other);
        return *this;
    }


    void clear()
    {
        raw.clear();
        formatted.clear();
    }

    std::string logger_name;
    level::level_enum level;
    log_clock::time_point time;
    std::tm tm_time;
    fast_oss raw;
    fast_oss formatted;


};
}
}
