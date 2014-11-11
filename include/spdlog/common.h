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

#include<initializer_list>
#include<chrono>

namespace spdlog
{
class formatter;
namespace sinks
{
class sink;
}

// Common types across the lib
using log_clock = std::chrono::system_clock;
using sink_ptr = std::shared_ptr < sinks::sink > ;
using sinks_init_list = std::initializer_list < sink_ptr > ;
using formatter_ptr = std::shared_ptr<spdlog::formatter>;

//Log level enum
namespace level
{
typedef enum
{
    TRACE    = 0,
    DEBUG    = 1,
    INFO     = 2,
    NOTICE   = 3,
    WARN     = 4, 
    ERR      = 5,
    CRITICAL = 6,
    ALERT    = 7,
    EMERG    = 8,
    ALWAYS   = 9,
    OFF      = 10
} level_enum;

static const char* level_names[] { "trace", "debug", "info", "notice", "warning", "error", "critical",
        "alert", "emerg", "", ""};
inline const char* to_str(spdlog::level::level_enum l)
{
    return level_names[l];
}
} //level

//
// Log exception
//
class spdlog_ex : public std::exception
{
public:
    spdlog_ex(const std::string& msg) :_msg(msg) {};
    const char* what() const throw() override
    {
        return _msg.c_str();
    }
private:
    std::string _msg;

};

} //spdlog
