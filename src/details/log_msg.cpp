// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/details/log_msg.h"

#include "spdlog/details/os.h"

namespace spdlog {
namespace details {

log_msg::log_msg(const log_clock::time_point log_time,
                 const source_loc &loc,
                 const string_view_t logger_name,
                 const level lvl,
                 const string_view_t msg,
                 const log_attributes attributes)
    : logger_name(logger_name),
      log_level(lvl),
      time(log_time),
#ifdef SPDLOG_NO_THREAD_ID
      thread_id(0),
#else
      thread_id(os::thread_id()),
#endif
      source(loc),
      payload(msg),
      attributes(attributes) {
}

log_msg::log_msg(const log_clock::time_point log_time,
                 const source_loc &loc,
                 const string_view_t logger_name,
                 const level lvl,
                 const string_view_t msg)
    : logger_name(logger_name),
      log_level(lvl),
      time(log_time),
#ifdef SPDLOG_NO_THREAD_ID
      thread_id(0),
#else
      thread_id(os::thread_id()),
#endif
      source(loc),
      payload(msg) {
}

log_msg::log_msg(const source_loc &loc, const string_view_t logger_name, const level lvl, const string_view_t msg)
    : log_msg(os::now(), loc, logger_name, lvl, msg, {}) {}

log_msg::log_msg(const source_loc &loc, string_view_t logger_name, level lvl, string_view_t msg, log_attributes attributes)
    : log_msg(os::now(), loc, logger_name, lvl, msg, attributes) {}

log_msg::log_msg(const string_view_t logger_name, const level lvl, const string_view_t msg)
    : log_msg(os::now(), source_loc{}, logger_name, lvl, msg) {}

log_msg::log_msg(const string_view_t logger_name, const level lvl, const string_view_t msg, const log_attributes attributes)
    : log_msg(os::now(), source_loc{}, logger_name, lvl, msg, attributes) {}

}  // namespace details
}  // namespace spdlog
