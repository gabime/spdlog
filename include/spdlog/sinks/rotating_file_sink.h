// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <mutex>
#include <optional>
#include <string>

#include "../details/file_helper.h"
#include "../details/null_mutex.h"
#include "./base_sink.h"

// Rotating file sink based on size

namespace spdlog {
namespace sinks {
template <typename Mutex>
class rotating_file_sink final : public base_sink<Mutex> {
public:
    using file_rotation_event_handler = std::function<std::optional<details::log_msg>(const filename_t &)>;

    rotating_file_sink(filename_t base_filename,
                       std::size_t max_size,
                       std::size_t max_files,
                       bool rotate_on_open = false,
                       const file_event_handlers &event_handlers = {},
                       file_rotation_event_handler file_rotating_event_callback = nullptr);

    rotating_file_sink(filename_t base_filename,
                       std::size_t max_size,
                       std::size_t max_files,
                       bool rotate_on_open,
                       file_rotation_event_handler file_rotating_event_callback);

    static filename_t calc_filename(const filename_t &filename, std::size_t index);
    filename_t filename();
    void rotate_now();

protected:
    void sink_it_(const details::log_msg &msg) override;
    void flush_() override;

private:
    // Rotate files:
    // log.txt -> log.1.txt
    // log.1.txt -> log.2.txt
    // log.2.txt -> log.3.txt
    // log.3.txt -> delete
    void rotate_();

    // delete the target if exists, and rename the src file  to target
    // return true on success, false otherwise.
    static bool rename_file_(const filename_t &src_filename, const filename_t &target_filename) noexcept;

    filename_t base_filename_;
    std::size_t max_size_;
    std::size_t max_files_;
    std::size_t current_size_;
    details::file_helper file_helper_;
    file_rotation_event_handler file_rotating_event_callback_;
};

using rotating_file_sink_mt = rotating_file_sink<std::mutex>;
using rotating_file_sink_st = rotating_file_sink<details::null_mutex>;

}  // namespace sinks
}  // namespace spdlog
