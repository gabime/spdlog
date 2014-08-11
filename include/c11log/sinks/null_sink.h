#pragma once

#include <mutex>
#include <memory>
#include "base_sink.h"

namespace c11log {
namespace sinks {
class null_sink : public base_sink
{
protected:
    void _sink_it(const details::log_msg&) override
    {}
};

}
}

