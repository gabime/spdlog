//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "../details/null_mutex.h"
#include "base_sink.h"

#include <mutex>
#include <ostream>

namespace spdlog {
namespace sinks {
template<class Mutex>
class ostream_sink : public base_sink<Mutex>
{
public:
    explicit ostream_sink(std::ostream &os, bool force_flush = false)
        : _ostream(os)
        , _force_flush(force_flush)
    {
    }
    ostream_sink(const ostream_sink &) = delete;
    ostream_sink &operator=(const ostream_sink &) = delete;

protected:
    void _sink_it(const details::log_msg &msg) override
    {
        _ostream.write(msg.formatted.data(), msg.formatted.size());
        if (_force_flush)
            _ostream.flush();
    }

    void _flush() override
    {
        _ostream.flush();
    }

    std::ostream &_ostream;
    bool _force_flush;
};

using ostream_sink_mt = ostream_sink<std::mutex>;
using ostream_sink_st = ostream_sink<details::null_mutex>;

} // namespace sinks
} // namespace spdlog
