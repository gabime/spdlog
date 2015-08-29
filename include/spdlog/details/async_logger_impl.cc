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
#ifdef SPDLOG_LIBRARY
#include "../async_logger.h"
#else
#pragma once
#endif


#include "./async_log_helper.h"
#include "./config.h"

SPDLOG_INLINE spdlog::async_logger::async_logger(const std::string& logger_name,
        sinks_init_list sinks,
        size_t queue_size,
        const  async_overflow_policy overflow_policy,
        const std::function<void()>& worker_warmup_cb,
        const std::chrono::milliseconds& flush_interval_ms) :
    async_logger(logger_name, sinks.begin(), sinks.end(), queue_size, overflow_policy, worker_warmup_cb, flush_interval_ms) {}

SPDLOG_INLINE spdlog::async_logger::async_logger(const std::string& logger_name,
        sink_ptr single_sink,
        size_t queue_size,
        const  async_overflow_policy overflow_policy,
        const std::function<void()>& worker_warmup_cb,
        const std::chrono::milliseconds& flush_interval_ms) :
    async_logger(logger_name, { single_sink }, queue_size, overflow_policy, worker_warmup_cb, flush_interval_ms) {}


SPDLOG_INLINE void spdlog::async_logger::_set_formatter(spdlog::formatter_ptr msg_formatter)
{
    _formatter = msg_formatter;
    _async_log_helper->set_formatter(_formatter);
}

SPDLOG_INLINE void spdlog::async_logger::_set_pattern(const std::string& pattern)
{
    _formatter = std::make_shared<pattern_formatter>(pattern);
    _async_log_helper->set_formatter(_formatter);
}


SPDLOG_INLINE void spdlog::async_logger::_log_msg(details::log_msg& msg)
{
    _async_log_helper->log(msg);
}
