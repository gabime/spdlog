/*
 * This content is released under the MIT License as specified in
 * https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"

using spdlog::details::os::create_dir;
using spdlog::details::os::path_exists;

bool try_create_dir(const spdlog::filename_t &path, const spdlog::filename_t &normalized_path) {
    auto rv = create_dir(path);
    REQUIRE(rv == true);
    return path_exists(normalized_path);
}

TEST_CASE("create_dir", "[create_dir]") {
    prepare_logdir();

    REQUIRE(try_create_dir(SPDLOG_FILENAME_T("test_logs/dir1/dir1"),
                           SPDLOG_FILENAME_T("test_logs/dir1/dir1")));
    REQUIRE(try_create_dir(SPDLOG_FILENAME_T("test_logs/dir1/dir1"),
                           SPDLOG_FILENAME_T("test_logs/dir1/dir1")));  // test existing
    REQUIRE(try_create_dir(SPDLOG_FILENAME_T("test_logs/dir1///dir2//"),
                           SPDLOG_FILENAME_T("test_logs/dir1/dir2")));
    REQUIRE(try_create_dir(SPDLOG_FILENAME_T("./test_logs/dir1/dir3"),
                           SPDLOG_FILENAME_T("test_logs/dir1/dir3")));
    REQUIRE(try_create_dir(SPDLOG_FILENAME_T("test_logs/../test_logs/dir1/dir4"),
                           SPDLOG_FILENAME_T("test_logs/dir1/dir4")));

#ifdef WIN32
    // test backslash folder separator
    REQUIRE(try_create_dir(SPDLOG_FILENAME_T("test_logs\\dir1\\dir222"),
                           SPDLOG_FILENAME_T("test_logs\\dir1\\dir222")));
    REQUIRE(try_create_dir(SPDLOG_FILENAME_T("test_logs\\dir1\\dir223\\"),
                           SPDLOG_FILENAME_T("test_logs\\dir1\\dir223\\")));
    REQUIRE(try_create_dir(SPDLOG_FILENAME_T(".\\test_logs\\dir1\\dir2\\dir99\\..\\dir23"),
                           SPDLOG_FILENAME_T("test_logs\\dir1\\dir2\\dir23")));
    REQUIRE(try_create_dir(SPDLOG_FILENAME_T("test_logs\\..\\test_logs\\dir1\\dir5"),
                           SPDLOG_FILENAME_T("test_logs\\dir1\\dir5")));
#endif
}

TEST_CASE("create_invalid_dir", "[create_dir]") {
    REQUIRE(create_dir(SPDLOG_FILENAME_T("")) == false);
    REQUIRE(create_dir(spdlog::filename_t{}) == false);
#ifdef __linux__
    REQUIRE(create_dir("/proc/spdlog-utest") == false);
#endif
}

TEST_CASE("path_exists", "[create_dir]") {
    REQUIRE(path_exists(SPDLOG_FILENAME_T("")) == false);
    REQUIRE(path_exists(SPDLOG_FILENAME_T("non_existing_path_xyz")) == false);
    prepare_logdir();
    REQUIRE(create_dir(SPDLOG_FILENAME_T("test_logs/dir1/dir2")) == true);
    REQUIRE(path_exists(SPDLOG_FILENAME_T("test_logs/dir1/dir2")) == true);
    REQUIRE(create_dir(SPDLOG_FILENAME_T("test_logs/dir1/dir2")) == true);  // already existing nested
    REQUIRE(create_dir(SPDLOG_FILENAME_T("test_logs/dir1/dir3///")) == true);  // trailing separator
    REQUIRE(path_exists(SPDLOG_FILENAME_T("test_logs/dir1/dir3")) == true);
}

TEST_CASE("dir_name", "[create_dir]") {
    using spdlog::details::os::dir_name;
    REQUIRE(dir_name(SPDLOG_FILENAME_T("")).empty());
    REQUIRE(dir_name(SPDLOG_FILENAME_T("dir")).empty());

#ifdef WIN32
    REQUIRE(dir_name(SPDLOG_FILENAME_T(R"(dir\)")) == SPDLOG_FILENAME_T("dir"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T(R"(dir\\\)")) == SPDLOG_FILENAME_T(R"(dir\\)"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T(R"(dir\file)")) == SPDLOG_FILENAME_T("dir"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T(R"(dir/file)")) == SPDLOG_FILENAME_T("dir"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T(R"(dir\file.txt)")) == SPDLOG_FILENAME_T("dir"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T(R"(dir/file)")) == SPDLOG_FILENAME_T("dir"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T(R"(dir\file.txt\)")) ==
            SPDLOG_FILENAME_T(R"(dir\file.txt)"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T(R"(\dir\file.txt)")) == SPDLOG_FILENAME_T(R"(\dir)"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T(R"(\\dir\file.txt)")) == SPDLOG_FILENAME_T(R"(\\dir)"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T(R"(..\file.txt)")) == SPDLOG_FILENAME_T(".."));
    REQUIRE(dir_name(SPDLOG_FILENAME_T(R"(.\file.txt)")) == SPDLOG_FILENAME_T("."));
    REQUIRE(dir_name(SPDLOG_FILENAME_T(R"(c:\\a\b\c\d\file.txt)")) ==
            SPDLOG_FILENAME_T(R"(c:\\a\b\c\d)"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T(R"(c://a/b/c/d/file.txt)")) ==
            SPDLOG_FILENAME_T(R"(c://a/b/c/d)"));
#endif
    REQUIRE(dir_name(SPDLOG_FILENAME_T("dir/")) == SPDLOG_FILENAME_T("dir"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T("dir///")) == SPDLOG_FILENAME_T("dir//"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T("dir/file")) == SPDLOG_FILENAME_T("dir"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T("dir/file.txt")) == SPDLOG_FILENAME_T("dir"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T("dir/file.txt/")) == SPDLOG_FILENAME_T("dir/file.txt"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T("/dir/file.txt")) == SPDLOG_FILENAME_T("/dir"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T("//dir/file.txt")) == SPDLOG_FILENAME_T("//dir"));
    REQUIRE(dir_name(SPDLOG_FILENAME_T("../file.txt")) == SPDLOG_FILENAME_T(".."));
    REQUIRE(dir_name(SPDLOG_FILENAME_T("./file.txt")) == SPDLOG_FILENAME_T("."));
}

#ifdef _WIN32

//
// test windows cases when drive letter is given e.g. C:\\some-folder
//
#include <windows.h>
#include <fileapi.h>

std::string get_full_path(const std::string &relative_folder_path) {
    char full_path[MAX_PATH];

    DWORD result = ::GetFullPathNameA(relative_folder_path.c_str(), MAX_PATH, full_path, nullptr);
    // Return an empty string if failed to get full path
    return result > 0 && result < MAX_PATH ? std::string(full_path) : std::string();
}

std::wstring get_full_path(const std::wstring &relative_folder_path) {
    wchar_t full_path[MAX_PATH];
    DWORD result = ::GetFullPathNameW(relative_folder_path.c_str(), MAX_PATH, full_path, nullptr);
    return result > 0 && result < MAX_PATH ? std::wstring(full_path) : std::wstring();
}

spdlog::filename_t::value_type find_non_existing_drive() {
    for (char drive = 'A'; drive <= 'Z'; ++drive) {
        std::string root_path = std::string(1, drive) + ":\\";
        UINT drive_type = GetDriveTypeA(root_path.c_str());
        if (drive_type == DRIVE_NO_ROOT_DIR) {
            return static_cast<spdlog::filename_t::value_type>(drive);
        }
    }
    return '\0';  // No available drive found
}

TEST_CASE("create_abs_path1", "[create_dir]") {
    prepare_logdir();
    auto abs_path = get_full_path(SPDLOG_FILENAME_T("test_logs\\logdir1"));
    REQUIRE(!abs_path.empty());
    REQUIRE(create_dir(abs_path) == true);
}

TEST_CASE("create_abs_path2", "[create_dir]") {
    prepare_logdir();
    auto abs_path = get_full_path(SPDLOG_FILENAME_T("test_logs/logdir2"));
    REQUIRE(!abs_path.empty());
    REQUIRE(create_dir(abs_path) == true);
}

TEST_CASE("non_existing_drive", "[create_dir]") {
    prepare_logdir();
    spdlog::filename_t path;

    auto non_existing_drive = find_non_existing_drive();
    path += non_existing_drive;
    path += SPDLOG_FILENAME_T(":\\");
    REQUIRE(create_dir(path) == false);
    path += SPDLOG_FILENAME_T("subdir");
    REQUIRE(create_dir(path) == false);
}
// #endif  // SPDLOG_WCHAR_FILENAMES
#endif  // _WIN32
