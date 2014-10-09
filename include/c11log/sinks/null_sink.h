#pragma once

#include "base_sink.h"

namespace c11log {
namespace sinks {

template <class Mutex>
class null_sink : public base_sink<Mutex>
{
protected:
    void _sink_it(const details::log_msg&) override
    {}
};

}
}

