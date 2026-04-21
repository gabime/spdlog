#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "includes.h"
#include "test_sink.h"

TEST_CASE("transactional_logging_basic", "[transactional_logging]") {
    using spdlog::sinks::test_sink_mt;
    auto logger = spdlog::create<test_sink_mt>("transactional_logger");
    auto test_sink = std::static_pointer_cast<test_sink_mt>(logger->sinks()[0]);
    logger->set_pattern("%v");

    logger->info("message 1");
    REQUIRE(test_sink->msg_counter() == 1);
    REQUIRE(test_sink->lines()[0] == "message 1");

    logger->start_transaction();
    REQUIRE(logger->in_transaction() == true);

    logger->info("message 2");
    logger->info("message 3");
    REQUIRE(test_sink->msg_counter() == 1);

    logger->commit_transaction();
    REQUIRE(logger->in_transaction() == false);
    REQUIRE(test_sink->msg_counter() == 3);
    REQUIRE(test_sink->lines()[1] == "message 2");
    REQUIRE(test_sink->lines()[2] == "message 3");

    spdlog::drop_all();
}

TEST_CASE("transactional_logging_rollback", "[transactional_logging]") {
    using spdlog::sinks::test_sink_mt;
    auto logger = spdlog::create<test_sink_mt>("transactional_logger");
    auto test_sink = std::static_pointer_cast<test_sink_mt>(logger->sinks()[0]);
    logger->set_pattern("%v");

    logger->info("message 1");
    REQUIRE(test_sink->msg_counter() == 1);

    logger->start_transaction();
    logger->info("message 2");
    logger->info("message 3");
    REQUIRE(test_sink->msg_counter() == 1);

    logger->rollback_transaction();
    REQUIRE(logger->in_transaction() == false);
    REQUIRE(test_sink->msg_counter() == 1);

    logger->info("message 4");
    REQUIRE(test_sink->msg_counter() == 2);
    REQUIRE(test_sink->lines()[1] == "message 4");

    spdlog::drop_all();
}

TEST_CASE("transactional_logging_nested_not_supported", "[transactional_logging]") {
    using spdlog::sinks::test_sink_mt;
    auto logger = spdlog::create<test_sink_mt>("transactional_logger");
    auto test_sink = std::static_pointer_cast<test_sink_mt>(logger->sinks()[0]);
    logger->set_pattern("%v");

    logger->start_transaction();
    logger->info("message 1");

    logger->start_transaction();
    logger->info("message 2");

    logger->commit_transaction();
    REQUIRE(test_sink->msg_counter() == 1);
    REQUIRE(test_sink->lines()[0] == "message 2");

    REQUIRE(logger->in_transaction() == true);

    logger->commit_transaction();
    REQUIRE(test_sink->msg_counter() == 2);
    REQUIRE(test_sink->lines()[1] == "message 1");

    spdlog::drop_all();
}

TEST_CASE("transactional_logging_multiple_loggers", "[transactional_logging]") {
    using spdlog::sinks::test_sink_mt;
    auto logger1 = spdlog::create<test_sink_mt>("logger1");
    auto logger2 = spdlog::create<test_sink_mt>("logger2");
    auto test_sink1 = std::static_pointer_cast<test_sink_mt>(logger1->sinks()[0]);
    auto test_sink2 = std::static_pointer_cast<test_sink_mt>(logger2->sinks()[0]);
    logger1->set_pattern("%v");
    logger2->set_pattern("%v");

    logger1->info("logger1 message 1");
    logger2->info("logger2 message 1");
    REQUIRE(test_sink1->msg_counter() == 1);
    REQUIRE(test_sink2->msg_counter() == 1);

    logger1->start_transaction();
    REQUIRE(logger1->in_transaction() == true);
    REQUIRE(logger2->in_transaction() == false);

    logger1->info("logger1 message 2");
    logger2->info("logger2 message 2");
    REQUIRE(test_sink1->msg_counter() == 1);
    REQUIRE(test_sink2->msg_counter() == 2);

    logger1->commit_transaction();
    REQUIRE(test_sink1->msg_counter() == 2);
    REQUIRE(test_sink1->lines()[1] == "logger1 message 2");

    spdlog::drop_all();
}

TEST_CASE("transactional_logging_exception_safety", "[transactional_logging]") {
    using spdlog::sinks::test_sink_mt;
    auto logger = spdlog::create<test_sink_mt>("transactional_logger");
    auto test_sink = std::static_pointer_cast<test_sink_mt>(logger->sinks()[0]);
    logger->set_pattern("%v");

    logger->info("before transaction");
    REQUIRE(test_sink->msg_counter() == 1);

    try {
        logger->start_transaction();
        logger->info("during transaction");
        throw std::runtime_error("test exception");
    } catch (...) {
        logger->rollback_transaction();
    }

    REQUIRE(logger->in_transaction() == false);
    REQUIRE(test_sink->msg_counter() == 1);

    logger->info("after rollback");
    REQUIRE(test_sink->msg_counter() == 2);
    REQUIRE(test_sink->lines()[1] == "after rollback");

    spdlog::drop_all();
}
