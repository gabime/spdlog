/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"

#define TEST_FILENAME "test_logs/file_helper_test.txt"

using spdlog::details::file_helper;

static void write_with_helper(file_helper &helper, size_t howmany)
{
    spdlog::memory_buf_t formatted;
    fmt::format_to(formatted, "{}", std::string(howmany, '1'));
    helper.write(formatted);
    helper.flush();
}

TEST_CASE("file_helper_filename", "[file_helper::filename()]]")
{
    prepare_logdir();

    file_helper helper;
    spdlog::filename_t target_filename = SPDLOG_FILENAME_T(TEST_FILENAME);
    helper.open(target_filename);
    REQUIRE(helper.filename() == target_filename);
}

TEST_CASE("file_helper_size", "[file_helper::size()]]")
{
    prepare_logdir();
    spdlog::filename_t target_filename = SPDLOG_FILENAME_T(TEST_FILENAME);
    size_t expected_size = 123;
    {
        file_helper helper;
        helper.open(target_filename);
        write_with_helper(helper, expected_size);
        REQUIRE(static_cast<size_t>(helper.size()) == expected_size);
    }
    REQUIRE(get_filesize(TEST_FILENAME) == expected_size);
}

TEST_CASE("file_helper_reopen", "[file_helper::reopen()]]")
{
    prepare_logdir();
    spdlog::filename_t target_filename = SPDLOG_FILENAME_T(TEST_FILENAME);
    file_helper helper;
    helper.open(target_filename);
    write_with_helper(helper, 12);
    REQUIRE(helper.size() == 12);
    helper.reopen(true);
    REQUIRE(helper.size() == 0);
}

TEST_CASE("file_helper_reopen2", "[file_helper::reopen(false)]]")
{
    prepare_logdir();
    spdlog::filename_t target_filename = SPDLOG_FILENAME_T(TEST_FILENAME);
    size_t expected_size = 14;
    file_helper helper;
    helper.open(target_filename);
    write_with_helper(helper, expected_size);
    REQUIRE(helper.size() == expected_size);
    helper.reopen(false);
    REQUIRE(helper.size() == expected_size);
}

static void test_split_ext(const spdlog::filename_t::value_type *fname, const spdlog::filename_t::value_type *expect_base, const spdlog::filename_t::value_type *expect_ext)
{
    spdlog::filename_t filename(fname);
    spdlog::filename_t expected_base(expect_base);
    spdlog::filename_t expected_ext(expect_ext);

    spdlog::filename_t basename;
    spdlog::filename_t ext;
    std::tie(basename, ext) = file_helper::split_by_extension(filename);
    REQUIRE(basename == expected_base);
    REQUIRE(ext == expected_ext);
}

TEST_CASE("file_helper_split_by_extension", "[file_helper::split_by_extension()]]")
{
    test_split_ext(SPDLOG_FILENAME_T("mylog.txt"), SPDLOG_FILENAME_T("mylog"), SPDLOG_FILENAME_T(".txt"));
    test_split_ext(SPDLOG_FILENAME_T(".mylog.txt"), SPDLOG_FILENAME_T(".mylog"), SPDLOG_FILENAME_T(".txt"));
    test_split_ext(SPDLOG_FILENAME_T(".mylog"), SPDLOG_FILENAME_T(".mylog"), SPDLOG_FILENAME_T(""));
    test_split_ext(SPDLOG_FILENAME_T("/aaa/bb.d/mylog"), SPDLOG_FILENAME_T("/aaa/bb.d/mylog"), SPDLOG_FILENAME_T(""));
    test_split_ext(SPDLOG_FILENAME_T("/aaa/bb.d/mylog.txt"), SPDLOG_FILENAME_T("/aaa/bb.d/mylog"), SPDLOG_FILENAME_T(".txt"));
    test_split_ext(SPDLOG_FILENAME_T("aaa/bbb/ccc/mylog.txt"), SPDLOG_FILENAME_T("aaa/bbb/ccc/mylog"), SPDLOG_FILENAME_T(".txt"));
    test_split_ext(SPDLOG_FILENAME_T("aaa/bbb/ccc/mylog."), SPDLOG_FILENAME_T("aaa/bbb/ccc/mylog."), SPDLOG_FILENAME_T(""));
    test_split_ext(SPDLOG_FILENAME_T("aaa/bbb/ccc/.mylog.txt"), SPDLOG_FILENAME_T("aaa/bbb/ccc/.mylog"), SPDLOG_FILENAME_T(".txt"));
    test_split_ext(SPDLOG_FILENAME_T("/aaa/bbb/ccc/mylog.txt"), SPDLOG_FILENAME_T("/aaa/bbb/ccc/mylog"), SPDLOG_FILENAME_T(".txt"));
    test_split_ext(SPDLOG_FILENAME_T("/aaa/bbb/ccc/.mylog"), SPDLOG_FILENAME_T("/aaa/bbb/ccc/.mylog"), SPDLOG_FILENAME_T(""));
    test_split_ext(SPDLOG_FILENAME_T("../mylog.txt"), SPDLOG_FILENAME_T("../mylog"), SPDLOG_FILENAME_T(".txt"));
    test_split_ext(SPDLOG_FILENAME_T(".././mylog.txt"), SPDLOG_FILENAME_T(".././mylog"), SPDLOG_FILENAME_T(".txt"));
    test_split_ext(SPDLOG_FILENAME_T(".././mylog.txt/xxx"), SPDLOG_FILENAME_T(".././mylog.txt/xxx"), SPDLOG_FILENAME_T(""));
    test_split_ext(SPDLOG_FILENAME_T("/mylog.txt"), SPDLOG_FILENAME_T("/mylog"), SPDLOG_FILENAME_T(".txt"));
    test_split_ext(SPDLOG_FILENAME_T("//mylog.txt"), SPDLOG_FILENAME_T("//mylog"), SPDLOG_FILENAME_T(".txt"));
    test_split_ext(SPDLOG_FILENAME_T(""), SPDLOG_FILENAME_T(""), SPDLOG_FILENAME_T(""));
    test_split_ext(SPDLOG_FILENAME_T("."), SPDLOG_FILENAME_T("."), SPDLOG_FILENAME_T(""));
    test_split_ext(SPDLOG_FILENAME_T("..txt"), SPDLOG_FILENAME_T("."), SPDLOG_FILENAME_T(".txt"));
}
