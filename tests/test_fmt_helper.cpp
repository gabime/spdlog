
#include "includes.h"

void test_pad2(int n, const char *expected)
{
    fmt::memory_buffer buf;
    spdlog::details::fmt_helper::pad2(n, buf);
    REQUIRE(fmt::to_string(buf) == expected);
}

void test_pad3(int n, const char *expected)
{
    fmt::memory_buffer buf;
    spdlog::details::fmt_helper::pad3(n, buf);
    REQUIRE(fmt::to_string(buf) == expected);
}

void test_pad6(std::size_t n, const char *expected)
{
    fmt::memory_buffer buf;
    spdlog::details::fmt_helper::pad6(n, buf);
    REQUIRE(fmt::to_string(buf) == expected);
}

TEST_CASE("pad2", "[fmt_helper]")
{
    test_pad2(0, "00");
    test_pad2(3, "03");
    test_pad2(23, "23");
    test_pad2(123, "123");
    test_pad2(1234, "1234");
    test_pad2(-5, "-5");
}

TEST_CASE("pad3", "[fmt_helper]")
{
    test_pad3(0, "000");
    test_pad3(3, "003");
    test_pad3(23, "023");
    test_pad3(123, "123");
    test_pad3(1234, "1234");
    test_pad3(-5, "-05");
}

TEST_CASE("pad6", "[fmt_helper]")
{
    test_pad6(0, "000000");
    test_pad6(3, "000003");
    test_pad6(23, "000023");
    test_pad6(123, "000123");
    test_pad6(1234, "001234");
    test_pad6(12345, "012345");
    test_pad6(123456, "123456");
}
