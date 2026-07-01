// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <filesystem>
#include <string>

#include "spdlog/common.h"
#include "spdlog/details/os.h"

SPDLOG_NAMESPACE_BEGIN
namespace details {
namespace os {

bool remove(const filename_t &filename) { return std::filesystem::remove(filename); }

bool remove_if_exists(const filename_t &filename) {
    if (path_exists(filename)) {
        return os::remove(filename);
    }
    return false;
}

// Rename if regular file
bool rename(const filename_t &filename1, const filename_t &filename2) noexcept {
    if (!std::filesystem::is_regular_file(filename1)) {
        return false;
    }
    std::error_code ec;
    std::filesystem::rename(filename1, filename2, ec);
    return !ec;
}

// Return true if path exists (file or directory)
bool path_exists(const filename_t &filename) noexcept { return std::filesystem::exists(filename); }

// Return directory name from given path or empty string
// "abc/file" => "abc"
// "abc/" => "abc"
// "abc" => ""
// "abc///" => "abc//"
filename_t dir_name(const filename_t &path) { return path.parent_path(); }

// Create the given directory - and all directories leading to it
// return true on success or if the directory already exists
bool create_dir(const filename_t &path) {
    std::error_code ec;
    return std::filesystem::create_directories(path, ec) || !ec;
}

// Return file path and its extension:
//
// "mylog.txt" => ("mylog", ".txt")
// "mylog" => ("mylog", "")
// "mylog." => ("mylog", ".")
// "/dir1/dir2/mylog.txt" => ("/dir1/dir2/mylog", ".txt")
// ".mylog" => (".mylog", "")
std::tuple<filename_t, filename_t> split_by_extension(const filename_t &fname) {
    const auto ext = fname.extension();
    auto without_ext = filename_t(fname).replace_extension();
    return std::make_tuple(without_ext, ext);
}
}  // namespace os
}  // namespace details
SPDLOG_NAMESPACE_END
