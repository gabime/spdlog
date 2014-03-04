#pragma once

#include <iostream>
#include <mutex>
#include <memory>

#include "base_sink.h"

namespace c11log {
namespace sinks {
class ostream_sink: public base_sink {
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

inline std::shared_ptr<ostream_sink> cout_sink() {
	static const ostream_sink& instance{std::cout};
	return std::shared_ptr<ostream_sink>(&instance, [=](ostream_sink*) {});
}

inline std::shared_ptr<ostream_sink> cerr_sink() {
	static const ostream_sink& instance = ostream_sink(std::cerr);
	return std::shared_ptr<ostream_sink>(&instance, [=](ostream_sink*) {});
}


}
}
