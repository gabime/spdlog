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

// Async logger
// Upon each log write the logger:
// 1. Checks if its log level is enough to log the message
// 2. Push a new copy of the message to a queue (uses sinks::async_sink for this)

#include <chrono>
#include "common.h"
#include "logger.h"


namespace spdlog
{

namespace sinks {
class async_sink;
}

class async_logger :public logger
{
public:
    template<class It>
    async_logger(const std::string& name, const It& begin, const It& end, size_t queue_size, const log_clock::duration& shutdown_duration);
    async_logger(const std::string& logger_name, sinks_init_list sinks, size_t queue_size, const log_clock::duration& shutdown_duration);
    async_logger(const std::string& logger_name, sink_ptr single_sink, size_t queue_size, const log_clock::duration& shutdown_duration);


protected:
    void _log_msg(details::log_msg& msg) override;
    void _set_formatter(spdlog::formatter_ptr msg_formatter) override;
    void _set_pattern(const std::string& pattern) override;
    void _stop() override;

private:
    std::unique_ptr<sinks::async_sink> _as;
    log_clock::duration _shutdown_duration;
};
}


#include "./details/async_logger_impl.h"
