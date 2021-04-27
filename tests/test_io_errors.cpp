/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#ifdef SPDLOG_NO_EXCEPTIONS
#error
#endif

#define TEST_FILENAME "test_logs/file_helper_test.txt"
using spdlog::details::file_helper;
using Catch::Matchers::Contains;

static int mocked_fwrite_errno;
extern "C" int __real_fwrite(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern "C" int __wrap_fwrite(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (mocked_fwrite_errno)
    {
        errno = mocked_fwrite_errno;
        mocked_fwrite_errno = 0;
        return 0;
    }
    return __real_fwrite(ptr, size, nmemb, stream);
}

static int mocked_fflush_errno;
extern "C" int __real_fflush(FILE *stream);
extern "C" int __wrap_fflush(FILE *stream)
{
    if (mocked_fflush_errno)
    {
        errno = mocked_fflush_errno;
        mocked_fflush_errno = 0;
        return EOF;
    }
    return __real_fflush(stream);
}

TEST_CASE("file_helper_write_error", "[file_helper::write()]]")
{
    spdlog::memory_buf_t buffer;
    fmt::format_to(buffer, "not empty");
    prepare_logdir();
    file_helper helper;
    helper.open(SPDLOG_FILENAME_T(TEST_FILENAME));
    mocked_fwrite_errno = EIO;
    CHECK_THROWS(helper.write(buffer));
    mocked_fwrite_errno = 0;
}

TEST_CASE("file_helper_flush_error", "[file_helper::flush()]]")
{
    prepare_logdir();
    file_helper helper;
    helper.open(SPDLOG_FILENAME_T(TEST_FILENAME));
    mocked_fflush_errno = EIO;
    CHECK_THROWS(helper.flush());
    mocked_fflush_errno = 0;
}

TEST_CASE("stdout_st_flush_error", "[stdout]")
{
    auto last_error_msg = std::string{};
    auto l = spdlog::stdout_logger_st("test");
    l->set_error_handler([&](const std::string &msg) { last_error_msg = msg; });
    l->set_pattern("%+");
    mocked_fflush_errno = EIO;
    l->info("Test stdout_st fflush error");
    mocked_fflush_errno = 0;
    spdlog::drop_all();
    REQUIRE_THAT(last_error_msg, Contains("flush"));
    REQUIRE_THAT(last_error_msg, Contains("base"));
}

TEST_CASE("stderr_st_write_error", "[stdout]")
{
    auto last_error_msg = std::string{};
    auto l = spdlog::stderr_logger_mt("test");
    l->set_error_handler([&](const std::string &msg) { last_error_msg = msg; });
    mocked_fwrite_errno = EIO;
    l->info("Test stderr_mt fwrite error");
    mocked_fwrite_errno = 0;
    spdlog::drop_all();
    REQUIRE_THAT(last_error_msg, Contains("write"));
    REQUIRE_THAT(last_error_msg, Contains("base"));
}

TEST_CASE("stdout_color_mt_flush_error", "[stdout]")
{
    auto last_error_msg = std::string{};
    auto l = spdlog::stdout_color_mt("test");
    l->set_error_handler([&](const std::string &msg) { last_error_msg = msg; });
    l->set_pattern("%+");
    mocked_fflush_errno = EIO;
    l->info("Test stdout_color_mt fflush error");
    mocked_fflush_errno = 0;
    spdlog::drop_all();
    REQUIRE_THAT(last_error_msg, Contains("flush"));
    REQUIRE_THAT(last_error_msg, Contains("ansi"));
}

TEST_CASE("stderr_color_st_write_error", "[stdout]")
{
    auto last_error_msg = std::string{};
    auto l = spdlog::stderr_color_st("test");
    l->set_error_handler([&](const std::string &msg) { last_error_msg = msg; });
    mocked_fwrite_errno = EIO;
    l->info("Test stderr_color_st fwrite error");
    mocked_fwrite_errno = 0;
    spdlog::drop_all();
    REQUIRE_THAT(last_error_msg, Contains("write"));
    REQUIRE_THAT(last_error_msg, Contains("ansi"));
}
