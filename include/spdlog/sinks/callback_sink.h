//
// Copyright (c) 2016 Kevin M. Godby.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

//
// This sink will call a callback function any time it's told to log something.
//

#include <spdlog/sinks/base_sink.h>
#include <spdlog/common.h>
#include <spdlog/details/log_msg.h>

#include <mutex>

namespace spdlog {
namespace sinks {

typedef void(*LogCallback)(void* /*userdata*/,
                           const char* /*logger_name*/,
                           int /*level*/,
                           size_t /*thread_id*/,
                           const char* /*msg*/);


template<class Mutex>
class callback_sink : public base_sink<Mutex> {
public:
    callback_sink(LogCallback callback = nullptr, void* userdata = nullptr) : _callback(callback), _userdata(userdata)
    {
        // do nothing
    }

    // noncopyable
    callback_sink(const callback_sink&) = delete;
    callback_sink& operator=(const callback_sink&) = delete;

    virtual ~callback_sink() = default;

    virtual void flush() override
    {
        // do nothing
    }

    void set_callback(LogCallback& callback, void* userdata)
    {
        _callback = callback;
        _userdata = userdata;
    }

protected:
    void _sink_it(const details::log_msg& msg) override
    {
        if (!_callback)
            return;

        _callback(_userdata, msg.logger_name.c_str(),
                  static_cast<int>(msg.level), msg.thread_id,
                  msg.formatted.c_str());
    }

    LogCallback _callback;
    void* _userdata;
};

typedef callback_sink<std::mutex> callback_sink_mt;
typedef callback_sink<details::null_mutex> callback_sink_st;

} // end namespace sinks
} // end namespace spdlog

