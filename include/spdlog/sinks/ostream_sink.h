#pragma once

#include <iostream>
#include <mutex>
#include <memory>

#include "../details/null_mutex.h"
#include "./base_sink.h"

namespace spdlog
{
namespace sinks
{
template<class Mutex>
class ostream_sink: public base_sink<Mutex>
{
public:
    explicit ostream_sink(std::ostream& os) :_ostream(os) {}
    ostream_sink(const ostream_sink&) = delete;
    ostream_sink& operator=(const ostream_sink&) = delete;
    virtual ~ostream_sink() = default;


    void close() override
    {}

protected:
    virtual void _sink_it(const details::log_msg& msg) override
    {
        auto& buf = msg.formatted.buf();
        _ostream.write(buf.data(), buf.size());
    }
    std::ostream& _ostream;
};

typedef ostream_sink<std::mutex> ostream_sink_mt;
typedef ostream_sink<details::null_mutex> ostream_sink_st;
}
}
