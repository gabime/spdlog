/*
* This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
*/
#include "includes.h"

using namespace spdlog::details;

static const std::string target_filename = "logs/file_helper_test.txt";

static void write_with_helper(file_helper &helper, size_t howmany)
{
    log_msg msg;
    msg.formatted << std::string(howmany, '1');
    helper.write(msg);
    helper.flush();
}


TEST_CASE("file_helper_filename", "[file_helper::filename()]]")
{
    prepare_logdir();

    file_helper helper;
    helper.open(target_filename);
    REQUIRE(helper.filename() == target_filename);
}



TEST_CASE("file_helper_size", "[file_helper::size()]]")
{
    prepare_logdir();
    size_t expected_size = 123;
    {
        file_helper helper;
        helper.open(target_filename);
        write_with_helper(helper, expected_size);
        REQUIRE(static_cast<size_t>(helper.size()) == expected_size);
    }
    REQUIRE(get_filesize(target_filename) == expected_size);
}


TEST_CASE("file_helper_exists", "[file_helper::file_exists()]]")
{
    prepare_logdir();
    REQUIRE(!file_helper::file_exists(target_filename));
    file_helper helper;
    helper.open(target_filename);
    REQUIRE(file_helper::file_exists(target_filename));
}

TEST_CASE("file_helper_reopen", "[file_helper::reopen()]]")
{
    prepare_logdir();
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
    size_t expected_size = 14;
    file_helper helper;
    helper.open(target_filename);
    write_with_helper(helper, expected_size);
    REQUIRE(helper.size() == expected_size);
    helper.reopen(false);
    REQUIRE(helper.size() == expected_size);
}



static void test_split_ext(spdlog::filename_t& filename, spdlog::filename_t & expected_base, spdlog::filename_t & expected_ext)
{	

#ifdef _WIN32 // replace folder sep
		std::replace(filename.begin(), filename.end(), '/', '\\'); 
		std::replace(expected_base.begin(), expected_base.end(), '/', '\\');		
#endif
	spdlog::filename_t basename, ext;
	std::tie(basename, ext) = file_helper::split_by_extenstion(filename);		
	REQUIRE(basename == expected_base);
	REQUIRE(ext == expected_ext);	
}


TEST_CASE("file_helper_split_by_extenstion", "[file_helper::split_by_extenstion()]]")
{
	
	using file_t = spdlog::filename_t;
	test_split_ext(file_t("mylog.txt"), file_t("mylog"), file_t(".txt"));
	test_split_ext(file_t(".mylog.txt"), file_t(".mylog"), file_t(".txt"));
	test_split_ext(file_t(".mylog"), file_t(".mylog"), file_t(""));
	test_split_ext(file_t("/aaa/bb.d/mylog"), file_t("/aaa/bb.d/mylog"), file_t(""));
	test_split_ext(file_t("/aaa/bb.d/mylog.txt"), file_t("/aaa/bb.d/mylog"), file_t(".txt"));
	test_split_ext(file_t("aaa/bbb/ccc/mylog.txt"), file_t("aaa/bbb/ccc/mylog"), file_t(".txt"));
	test_split_ext(file_t("aaa/bbb/ccc/mylog."), file_t("aaa/bbb/ccc/mylog."), file_t(""));
	test_split_ext(file_t("aaa/bbb/ccc/.mylog.txt"), file_t("aaa/bbb/ccc/.mylog"), file_t(".txt"));
	test_split_ext(file_t("/aaa/bbb/ccc/mylog.txt"), file_t("/aaa/bbb/ccc/mylog"), file_t(".txt"));
	test_split_ext(file_t("/aaa/bbb/ccc/.mylog"), file_t("/aaa/bbb/ccc/.mylog"), file_t(""));
	test_split_ext(file_t("../mylog.txt"), file_t("../mylog"), file_t(".txt"));
	test_split_ext(file_t(".././mylog.txt"), file_t(".././mylog"), file_t(".txt"));
	test_split_ext(file_t(".././mylog.txt/xxx"), file_t(".././mylog.txt/xxx"), file_t(""));		
	test_split_ext(file_t("/mylog.txt"), file_t("/mylog"), file_t(".txt"));
	test_split_ext(file_t("//mylog.txt"), file_t("//mylog"), file_t(".txt"));
}



