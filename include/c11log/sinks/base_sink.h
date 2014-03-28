#pragma once

#include<string>
#include<atomic>

#include "../formatter.h"
#include "../common_types.h"

namespace c11log
{
namespace sinks
{
class base_sink
{
public:
    base_sink(): _enabled(true) {}
    virtual ~base_sink() = default;

    base_sink(const base_sink&) = delete;
    base_sink& operator=(const base_sink&) = delete;

    void log(const log_msg& msg)
    {
        if (_enabled)
        {
            _sink_it(msg);
        }
    };

    void enable(bool enabled)
    {
        _enabled = enabled;
    }

    bool is_enabled()
    {
        return _enabled.load();
    }

protected:
    virtual void _sink_it(const log_msg& msg) = 0;
    std::atomic<bool> _enabled;
};

class null_sink:public base_sink
{
protected:
    void _sink_it(const log_msg&) override
    {
    }
};
}
}
