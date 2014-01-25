#pragma once
#include<string>
#include<memory>
#include<mutex>

#include "../formatters/formatters.h"
#include "../level.h"

namespace c11log {
namespace sinks {
class base_sink {
public:
    base_sink()
    {}
    base_sink(level::level_enum l):_level(l)
    {};
    virtual ~base_sink()
    {};

    base_sink(const base_sink&) = delete;
    base_sink& operator=(const base_sink&) = delete;

    void log(const std::string &msg, level::level_enum level)
    {
        if (level >= _level) {
            std::lock_guard<std::mutex> lock(_mutex);
            if (level >= _level)
                _sink_it(msg);
        }
    };

    void set_level(level::level_enum level)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _level = level;
    }

protected:
    virtual void _sink_it(const std::string& msg) = 0;
    level::level_enum _level = level::INFO;
    std::mutex _mutex;
};

class null_sink:public base_sink {
protected:
    void _sink_it(const std::string& msg) override
    {}
};
}
}