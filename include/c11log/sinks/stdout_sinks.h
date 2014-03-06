#pragma once

#include <iostream>
#include <mutex>
#include <memory>

#include "base_sink.h"

namespace c11log
{
namespace sinks
{
class ostream_sink: public base_sink
{
public:
    explicit ostream_sink(std::ostream& os):_ostream(os) {}
    ostream_sink(const ostream_sink&) = delete;
    ostream_sink& operator=(const ostream_sink&) = delete;
    virtual ~ostream_sink() = default;

protected:
    virtual void _sink_it(const std::string& msg) override {
        std::lock_guard<std::mutex> lock(_mutex);
        _ostream << msg;
    }

    std::ostream& _ostream;
    std::mutex _mutex;
};



}
}
