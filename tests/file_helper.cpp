/*
* This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
*/
#include "includes.h"

using namespace spdlog::details;

static const std::string filename = "logs/file_helper_test.txt";

static void write_with_helper(file_helper &helper, size_t howmany)
{
    log_msg msg;
    msg.formatted << std::string(howmany, '1');
    helper.write(msg);
}


TEST_CASE("file_helper_filename", "[file_helper::filename()]]")
{
    prepare_logdir();

    file_helper helper(false);
    helper.open(filename);
    REQUIRE(helper.filename() == filename);
}



TEST_CASE("file_helper_size", "[file_helper::size()]]")
{
    prepare_logdir();
    auto expected_size = 123;
    {
        file_helper helper(true);
        helper.open(filename);
        write_with_helper(helper, expected_size);
        REQUIRE(helper.size() == expected_size);
    }
    REQUIRE(get_filesize(filename) == expected_size);
}


TEST_CASE("file_helper_exists", "[file_helper::file_exists()]]")
{
    prepare_logdir();
    REQUIRE(!file_helper::file_exists(filename));
    file_helper helper(false);
    helper.open(filename);
    REQUIRE(file_helper::file_exists(filename));
}

TEST_CASE("file_helper_reopen", "[file_helper::reopen()]]")
{
    prepare_logdir();
    file_helper helper(true);
    helper.open(filename);
    write_with_helper(helper, 12);
    REQUIRE(helper.size() == 12);
    helper.reopen(true);
    REQUIRE(helper.size() == 0);
}

TEST_CASE("file_helper_reopen2", "[file_helper::reopen(false)]]")
{
    prepare_logdir();
    auto expected_size = 14;
    file_helper helper(true);
    helper.open(filename);
    write_with_helper(helper, expected_size);
    REQUIRE(helper.size() == expected_size);
    helper.reopen(false);
    REQUIRE(helper.size() == expected_size);
}




