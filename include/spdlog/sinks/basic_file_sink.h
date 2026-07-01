// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <mutex>
#include <string>

#include "../details/file_helper.h"
#include "../details/null_mutex.h"
#include "./base_sink.h"

SPDLOG_NAMESPACE_BEGIN
namespace sinks {
/*
 * Trivial file sink with single file as target
 */
template <typename Mutex>
class basic_file_sink final : public base_sink<Mutex> {
public:
    explicit basic_file_sink(const filename_t &filename, bool truncate = false, const file_event_handlers &event_handlers = {});
    const filename_t &filename() const;
    void truncate();

protected:
    void sink_it_(const details::log_msg &msg) override;
    void flush_() override;

private:
    details::file_helper file_helper_;
};

using basic_file_sink_mt = basic_file_sink<std::mutex>;
using basic_file_sink_st = basic_file_sink<details::null_mutex>;

}  // namespace sinks
SPDLOG_NAMESPACE_END
