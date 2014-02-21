#pragma once

#include <iostream>
#include "base_sink.h"

namespace c11log {
namespace sinks {
class ostream_sink: public base_sink {
public:
    ostream_sink(std::ostream& os):_ostream(os) {}
    virtual ~ostream_sink() = default;

protected:
    virtual void sink_it_(const std::string& msg) override {
        _ostream << msg;
    }

    std::ostream& _ostream;
};

class stdout_sink:public ostream_sink {
public:
    stdout_sink():ostream_sink(std::cout) {}
};

class stderr_sink:public ostream_sink {
public:
    stderr_sink():ostream_sink(std::cerr) {}

};
}
}