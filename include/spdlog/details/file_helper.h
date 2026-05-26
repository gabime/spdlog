// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <tuple>

#include "../common.h"
#include "../file_event_handlers.h"

SPDLOG_NAMESPACE_BEGIN
namespace details {

// Helper class for file sinks.
// When failing to open a file, retry several times(5) with a delay interval(10 ms).
// Throw spdlog_ex exception on errors.

class SPDLOG_API file_helper {
public:
    file_helper() = default;
    explicit file_helper(file_event_handlers event_handlers);

    file_helper(const file_helper &) = delete;
    file_helper &operator=(const file_helper &) = delete;
    ~file_helper();

    void open(const filename_t &fname, bool truncate = false);
    void reopen(bool truncate);
    void flush() const;
    void sync() const;
    void close();
    void write(const memory_buf_t &buf) const;
    size_t size() const;
    const filename_t &filename() const;

private:
    const int open_tries_ = 5;
    const unsigned int open_interval_ = 10;
    std::FILE *fd_{nullptr};
    filename_t filename_;
    file_event_handlers event_handlers_;
};
}  // namespace details
SPDLOG_NAMESPACE_END
