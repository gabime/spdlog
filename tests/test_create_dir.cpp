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
