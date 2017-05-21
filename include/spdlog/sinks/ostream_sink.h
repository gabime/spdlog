//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "spdlog/details/null_mutex.h"
#include "spdlog/sinks/base_sink.h"

#include <ostream>
#include <mutex>

namespace spdlog
{
namespace sinks
{
template<class Mutex>
class ostream_sink: public base_sink<Mutex>
{
public:
    explicit ostream_sink(std::ostream& os, bool force_flush=false) :_ostream(os), _force_flush(force_flush) {}
    ostream_sink(const ostream_sink&) = delete;
    ostream_sink& operator=(const ostream_sink&) = delete;
    virtual ~ostream_sink() = default;

protected:
    void _sink_it(const details::log_msg& msg) override
    {
        _ostream.write(msg.formatted.data(), msg.formatted.size());
        if (_force_flush)
            _ostream.flush();
    }

    void _flush() override
    {
        _ostream.flush();
    }

    std::ostream& _ostream;
    bool _force_flush;
};

typedef ostream_sink<std::mutex> ostream_sink_mt;
typedef ostream_sink<details::null_mutex> ostream_sink_st;
}
}
