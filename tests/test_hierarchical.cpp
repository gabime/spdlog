#include "includes.h"
#include "test_sink.h"
#include "spdlog/async.h"

TEST_CASE("hierarchical1", "[hierarchical]")
{

    using spdlog::sinks::test_sink_st;
    auto test_sink_root = std::make_shared<test_sink_st>();
    auto test_sink_lvl1 = std::make_shared<test_sink_st>();
    auto test_sink_lvl3 = std::make_shared<test_sink_st>();
    auto test_sink_lvl4 = std::make_shared<test_sink_st>();
    auto test_sink_lvl5 = std::make_shared<test_sink_st>();

    auto logger_root = std::make_shared<spdlog::logger>("", test_sink_root);
    logger_root->set_pattern("root - %v");
    logger_root->set_level(spdlog::level::info);
    spdlog::set_default_logger(logger_root);


    auto logger_1 = std::make_shared<spdlog::logger>("1", test_sink_lvl1);
    spdlog::register_logger(logger_1);
    logger_1->set_pattern("1 - %v");
    logger_1->set_level(spdlog::level::debug);
    // no sink for logger 2
    auto logger_2 = std::make_shared<spdlog::logger>("1.2");
    spdlog::register_logger(logger_2);
    logger_2->set_level(spdlog::level::info);
    auto logger_3 = std::make_shared<spdlog::logger>("1.2.3", test_sink_lvl3);
    spdlog::register_logger(logger_3);
    logger_3->set_pattern("3 - %v");
    logger_3->set_level(spdlog::level::warn);
    auto logger_4 = std::make_shared<spdlog::logger>("1.2.3.4", test_sink_lvl4);
    spdlog::register_logger(logger_4);
    logger_4->set_pattern("4 - %v");
    logger_4->set_level(spdlog::level::err);
    // Has propagate false, therefore should not propagate messages to higher loggers
    auto logger_5 = std::make_shared<spdlog::logger>("1.2.3.4.5", test_sink_lvl5);
    spdlog::register_logger(logger_5);
    logger_5->set_pattern("5 - %v");
    logger_5->set_level(spdlog::level::debug);
    logger_5->set_propagate(false);

    logger_4->error("error 4");
    logger_4->info("info 4");
    logger_4->debug("debug 4");
    logger_4->warn("warn 4");

    logger_2->error("error 2");
    logger_2->warn("warn 2");

    logger_5->error("error 5");

    REQUIRE(test_sink_root->lines().size() == 5);
    REQUIRE(test_sink_root->lines()[0] == "root - error 4");
    REQUIRE(test_sink_root->lines()[1] == "root - info 4");
    REQUIRE(test_sink_root->lines()[2] == "root - warn 4");
    REQUIRE(test_sink_root->lines()[3] == "root - error 2");
    REQUIRE(test_sink_root->lines()[4] == "root - warn 2");

    REQUIRE(test_sink_lvl1->lines().size() == 6);
    REQUIRE(test_sink_lvl1->lines()[0] == "1 - error 4");
    REQUIRE(test_sink_lvl1->lines()[1] == "1 - info 4");
    REQUIRE(test_sink_lvl1->lines()[2] == "1 - debug 4");
    REQUIRE(test_sink_lvl1->lines()[3] == "1 - warn 4");
    REQUIRE(test_sink_lvl1->lines()[4] == "1 - error 2");
    REQUIRE(test_sink_lvl1->lines()[5] == "1 - warn 2");

    REQUIRE(test_sink_lvl3->lines().size() == 2);
    REQUIRE(test_sink_lvl3->lines()[0] == "3 - error 4");
    REQUIRE(test_sink_lvl3->lines()[1] == "3 - warn 4");

    REQUIRE(test_sink_lvl4->lines().size() == 1);
    REQUIRE(test_sink_lvl4->lines()[0] == "4 - error 4");

    REQUIRE(test_sink_lvl5->lines().size() == 1);
    REQUIRE(test_sink_lvl5->lines()[0] == "5 - error 5");

}
