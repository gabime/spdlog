#pragma once

#include<string>
#include<mutex>
#include<atomic>
#include "isink.h"
#include "../formatter.h"
#include "../common.h"
#include "../details/log_msg.h"


namespace c11log
{
namespace sinks
{
template<class Mutex>
class base_sink:public isink
{
public:
    base_sink():_mutex(), _enabled(true) {}
    virtual ~base_sink() = default;

    base_sink(const base_sink&) = delete;
    base_sink& operator=(const base_sink&) = delete;

    void log(const details::log_msg& msg) override
    {
        if (_enabled)
        {
            std::lock_guard<Mutex> lock(_mutex);
            _sink_it(msg);
        }
    };

    void enable(bool enabled) override
    {
        _enabled = enabled;
    }

    bool is_enabled() override
    {
        return _enabled.load();
    }

protected:
    virtual void _sink_it(const details::log_msg& msg) = 0;
    Mutex _mutex;
    std::atomic<bool> _enabled;

};
}
}
