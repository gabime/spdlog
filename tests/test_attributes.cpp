// make attributes conform to spdlog requirements (thread-safe, exception-safe, etc)
#include "includes.h"
#include "test_sink.h"
#include <string>
#include <sstream>
#include <thread>

#define TEST_FILENAME "test_logs/attr_test.log"

// helper classes for exception-safe attribute push/pop
// these use RAII to ensure that the context is popped even if an exception is thrown
class scoped_context {
    std::shared_ptr<spdlog::logger> logger_;
    std::size_t push_counter_ = 0;

public:
    scoped_context(std::shared_ptr<spdlog::logger> logger) : logger_(logger) {}
    scoped_context(std::shared_ptr<spdlog::logger> logger, spdlog::attribute_list attrs) : scoped_context(logger) {
        push(attrs);
    }

    void push(spdlog::attribute_list attrs) {
        logger_->push_context(attrs);
        ++push_counter_;
    }

    ~scoped_context() {
        for (std::size_t i = 0; i < push_counter_; ++i) {
            logger_->pop_context();
        }
    }
};

TEST_CASE("attributes test with exceptions ", "[attributes]")
{
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("attr_logger", test_sink);
    logger->set_pattern("%v%( %K=%V%)");

    // push and pop context are not guaranteed to be exception safe
    // so we use the helper class
    try {
        auto ctx = scoped_context(logger, {{"key1", "val1"}});
        logger->info("testing");
        throw std::runtime_error("test exception");
    } catch (const std::exception& e) {
        logger->info("caught exception: {}", e.what());
    }

    REQUIRE(test_sink->msg_counter() == 2); 
    REQUIRE(test_sink->lines()[0] == "testing key1=val1"); // has kv pair in this msg
    REQUIRE(test_sink->lines()[1] == "caught exception: test exception"); // doesn't have kv pair in error msg
}

// see if multiple async logs to a single file is thread-safe, i.e. produces coherent structured logs
TEST_CASE("async attributes test with threads ", "[attributes]")
{
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_mt>();

    constexpr int num_loggers = 10;
    constexpr int num_msgs = 10;
    size_t overrun_counter = 0;

    {
    auto tp = std::make_shared<spdlog::details::thread_pool>(num_loggers * num_msgs, 1);
    std::vector<std::shared_ptr<spdlog::logger>> loggers;
    for (int i = 0; i < num_loggers; ++i) {
        loggers.push_back(std::make_shared<spdlog::async_logger>(
            "attr_logger_"+std::to_string(i), test_sink, tp, spdlog::async_overflow_policy::block));
    }

    std::vector<std::thread> threads;
    for (auto lg : loggers) {
        threads.emplace_back([=](){
            // push and pop context are not guaranteed to be thread safe
            // to guarantee thread safety, use a different logger object for each thread when using attributes
            for (int i = 0; i < num_msgs; ++i) {
                lg->push_context({{"key"+std::to_string(i), "val"+std::to_string(i)}});
                lg->info("testing {}", i);
                lg->pop_context();
            }
        });
    }
    for (auto& th : threads) {
        th.join();
    }

    for (auto lg : loggers) {
        lg->flush();
    }
    overrun_counter += tp->overrun_counter();
    }

    REQUIRE(test_sink->msg_counter() == num_loggers * num_msgs);
    REQUIRE(test_sink->flush_counter() == num_loggers);
    REQUIRE(overrun_counter == 0);

    // todo: better parsing of keys and values, maybe using regex
    for (auto& line : test_sink->lines()) {
        REQUIRE(line.find("key") != std::string::npos);
        REQUIRE(line.find("val") != std::string::npos);
    }
}

// testing custom patterns, including the new attribute patterns, with many use cases.
// some use cases may result in errors or undesirable behaviour, but the test should not crash.
TEST_CASE("pattern matching with attributes ", "[attributes]")
{
    auto test_sink = std::make_shared<spdlog::sinks::test_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("attr_logger", test_sink);
    std::string msg = "testing";
    logger->push_context({{"key1", "value1"}, {"key2", "value2"}});

    // a pattern with no stop flag will consider the stop flag as the end
    logger->set_pattern("%v%( %K=%V");
    logger->info(msg);

    REQUIRE(test_sink->lines()[test_sink->msg_counter()-1] == msg+" key1=value1 key2=value2");

    // a pattern with no start flag will be parsed as-is. the new flags will be treated as empty strings. 
    logger->set_pattern("%v %K=%V%) %v end");
    logger->info("testing");

    REQUIRE(test_sink->lines()[test_sink->msg_counter()-1] == msg+" = "+msg+" end");

    // a pattern with nested flags will print those flags for every key-value pair
    logger->set_pattern("%(%v %K=%V %)");
    logger->info("testing");
    REQUIRE(test_sink->lines()[test_sink->msg_counter()-1] == msg+" key1=value1 "+msg+" key2=value2 ");

    logger->set_pattern("%v start%( %K=%V%) %v end");
    // zero kv pairs
    logger->clear_context();
    logger->push_context({});
    logger->info(msg);

    REQUIRE(test_sink->lines()[test_sink->msg_counter()-1] == msg+" start "+msg+" end");

    // N kv pairs
    for (int i = 0; i < 50; ++i) {
        logger->push_context({{"key"+std::to_string(i), "value"+std::to_string(i)}});
        logger->info("testing");
        std::stringstream ss;
        ss << msg << " start";
        for (int j = 0; j <= i; ++j) {
            ss << " key" << j << "=value" << j;
        }
        ss << ' ' << msg << " end";

        REQUIRE(test_sink->lines()[test_sink->msg_counter()-1] == ss.str());
    }
}