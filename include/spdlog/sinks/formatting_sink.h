#pragma once

#include "base_sink.h"
#include "../common.h"

namespace spdlog {
namespace sinks {
/**
 * A sink decorator for sink-specific formatting. The incoming raw message is formatted, and passed to the backend sink.
 * Message formatting of the managing logger is ignored.
 * When using the thread-safe version of this class, the backend sink doesn't have to be a thread-safe type, since locking is already done by the decorator.
 **/
template<class Mutex>
class formatting_sink : public spdlog::sinks::base_sink<Mutex> {
public:
    formatting_sink(const spdlog::sink_ptr sink, const std::string& pattern, spdlog::pattern_time_type t = spdlog::pattern_time_type::local) : sink_(sink),
        formatter_(std::make_shared<spdlog::pattern_formatter>(pattern, t)) {
            set_level(sink->level());
        }

    ~formatting_sink() = default;

    inline void set_pattern(const std::string& pattern, spdlog::pattern_time_type t = spdlog::pattern_time_type::local) {
        formatter_ = std::make_shared<pattern_formatter>(pattern, pattern_time);
    }

    void set_formatter(spdlog::formatter_ptr msg_formatter) {
        formatter_ = msg_formatter;
    }

    void _sink_it(const spdlog::details::log_msg& msg) override {
        spdlog::details::log_msg myMsg(msg.logger_name, msg.level);
        myMsg.raw << msg.raw.c_str();
        formatter_->format(myMsg);
        sink_->log(myMsg);
    }

    void _flush() override {
        sink_->flush();
    }

protected:
    spdlog::sink_ptr sink_;
    spdlog::formatter_ptr formatter_;
};

typedef formatting_sink<std::mutex>                  formatting_sink_mt;
typedef formatting_sink<spdlog::details::null_mutex> formatting_sink_st;
}
}
