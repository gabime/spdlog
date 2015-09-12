/*************************************************************************/
/* spdlog - an extremely fast and easy to use c++11 logging library.     */
/* Copyright (c) 2015 Francois Coulombe.                                       */
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
#ifdef __ANDROID__
#include <ostream>
#include <mutex>
#include <memory>
#include <android/log.h>
#include "../details/null_mutex.h"
#include "./base_sink.h"


namespace spdlog
{
namespace sinks
{
template<class Mutex>
class android_sink: public base_sink<Mutex>
{
public:
    explicit android_sink(std::ostream& /*os*/, bool force_flush=false)
    ://_ostream(os),
    _force_flush(force_flush) {}
    android_sink(const android_sink&) = delete;
    android_sink& operator=(const android_sink&) = delete;
    virtual ~android_sink() = default;

protected:
    virtual void _sink_it(const details::log_msg& msg) override
    {
        constexpr int LogLevelMapping[] =
        {
            ANDROID_LOG_VERBOSE,
            ANDROID_LOG_DEBUG,
            ANDROID_LOG_INFO,
            ANDROID_LOG_INFO,
            ANDROID_LOG_WARN,
            ANDROID_LOG_ERROR,
            ANDROID_LOG_FATAL,
            ANDROID_LOG_FATAL,
            ANDROID_LOG_FATAL,
            ANDROID_LOG_SILENT,
        };
        ((void)__android_log_print(LogLevelMapping[msg.level], msg.logger_name.c_str(), msg.formatted.data()));
    }

    //std::ostream& _ostream;
    bool _force_flush;
};

typedef android_sink<std::mutex> android_sink_mt;
typedef android_sink<details::null_mutex> android_sink_st;
}
}
#endif
