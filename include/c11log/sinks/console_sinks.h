#pragma once

#include <iostream>
#include <mutex>
#include <memory>

#include "base_sink.h"

namespace c11log
{
namespace sinks
{
class console_sink: public base_sink
{
public:
    explicit console_sink(std::ostream& os):_ostream(os) {}
    console_sink(const console_sink&) = delete;
    console_sink& operator=(const console_sink&) = delete;
    virtual ~console_sink() = default;


protected:
    virtual void _sink_it(const details::log_msg& msg) override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _ostream.write(msg.msg_buf.first, msg.msg_buf.second);
    }

    std::ostream& _ostream;
    std::mutex _mutex;
};


inline std::shared_ptr<console_sink>& stdout_sink ()
{
    static auto inst = std::make_shared<console_sink>(std::cout);
    return inst;
}

inline std::shared_ptr<console_sink>& stderr_sink ()
{
    static auto inst = std::make_shared<console_sink>(std::cerr);
    return inst;
}

}
}
