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
    base_sink() = default;
    base_sink(level::level_enum l):_level(l)
    {
    };
    virtual ~base_sink() = default;

    base_sink(const base_sink&) = delete;
    base_sink& operator=(const base_sink&) = delete;

    void log(const bufpair_t &msg, level::level_enum level)
    {
        if (level >= _level)
        {
            _sink_it(msg);
        }
    };

    void set_level(level::level_enum level)
    {
        _level = level;
    }

protected:
    virtual void _sink_it(const bufpair_t& msg) = 0;
    std::atomic<int> _level {level::INFO};
};

class null_sink:public base_sink
{
protected:
    void _sink_it(const bufpair_t& ) override
    {
    }
};
}
}
