// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/common.h>
#include <tuple>
#include <vector>

SPDLOG_NAMESPACE_BEGIN
namespace details {

// Helper class for file sinks.
// When failing to open a file, retry several times(5) with a delay interval(10 ms).
// Throw spdlog_ex exception on errors.

class SPDLOG_API file_helper {
public:
    file_helper() = default;
    explicit file_helper(const file_event_handlers &event_handlers);

    file_helper(const file_helper &) = delete;
    file_helper &operator=(const file_helper &) = delete;
    ~file_helper();

    void open(const filename_t &fname, bool truncate = false);
    void reopen(bool truncate);
    void flush();
    void sync();
    void close();
    void write(const memory_buf_t &buf);
    size_t size() const;
    const filename_t &filename() const;

    // Set custom FILE stream buffer size via setvbuf(_IOFBF).
    // size == 0 clears the custom buffer and restores default buffering if the file is open.
    // Re-applied automatically on open/reopen when size > 0.
    void set_buffer(std::size_t size);

    //
    // return file path and its extension:
    //
    // "mylog.txt" => ("mylog", ".txt")
    // "mylog" => ("mylog", "")
    // "mylog." => ("mylog.", "")
    // "/dir1/dir2/mylog.txt" => ("/dir1/dir2/mylog", ".txt")
    //
    // the starting dot in filenames is ignored (hidden files):
    //
    // ".mylog" => (".mylog". "")
    // "my_folder/.mylog" => ("my_folder/.mylog", "")
    // "my_folder/.mylog.txt" => ("my_folder/.mylog", ".txt")
    static std::tuple<filename_t, filename_t> split_by_extension(const filename_t &fname);

private:
    void apply_buffer_();

    const int open_tries_ = 5;
    const unsigned int open_interval_ = 10;
    std::FILE *fd_{nullptr};
    filename_t filename_;
    file_event_handlers event_handlers_;
    std::vector<char> buffer_;
};
}  // namespace details
SPDLOG_NAMESPACE_END

#ifdef SPDLOG_HEADER_ONLY
#include "file_helper-inl.h"
#endif
