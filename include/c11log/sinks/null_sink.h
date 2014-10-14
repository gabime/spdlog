#pragma once
#include <mutex>
#include "base_sink.h"
#include "../details/null_mutex.h"


namespace c11log {
namespace sinks {

template <class Mutex>
class null_sink : public base_sink<Mutex>
{
protected:
    void _sink_it(const details::log_msg&) override
    {}
};

typedef null_sink<details::null_mutex> null_sink_st;
typedef null_sink<std::mutex> null_sink_mt;

}
}

