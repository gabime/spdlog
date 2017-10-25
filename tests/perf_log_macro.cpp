
#include <chrono>
#include <iostream>
#include <memory>
#include <sstream>
#include "includes.h"
#include <spdlog/fmt/ostr.h>

// Uncomment below to see timing calculations printed to stdout
//#define PERF_LOG_MACRO_PRINT_DURATIONS

struct MyObj
{
  MyObj()
    : val(0)
    , a_long_string("AAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC")
    , stream_run(false)
  {}
  int val;
  std::string a_long_string;
  mutable bool stream_run;
};
std::ostream& operator<<(std::ostream& os, const MyObj& c)
{
  c.stream_run = true;
  return os << c.a_long_string << " a " << c.val;
}

TEST_CASE("PERF log macro does not run the ostream", "[PERF_log_macro]")
{
  MyObj obj;
  auto null_sink = std::make_shared<spdlog::sinks::null_sink_st>();
  auto logger = std::make_shared<spdlog::logger>("null_logger", null_sink);
  logger->set_level(spdlog::level::info);
  SPDLOG_PERF_TRACE(logger, "another obj, {}", obj);
  logger->set_level(spdlog::level::trace);
  CHECK(!obj.stream_run);
  SPDLOG_PERF_TRACE(logger, "another obj, {}", obj);
  CHECK(obj.stream_run);
}

TEST_CASE("performace test when writing suing macro", "[PERF_log_macro]")
{
  {
    MyObj obj;
    auto null_sink = std::make_shared<spdlog::sinks::null_sink_st>();
    auto logger = std::make_shared<spdlog::logger>("null_logger", null_sink);
    logger->set_level(spdlog::level::info);

    std::size_t loop_count = 1000;
    auto begin = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < loop_count; i++)
    {
      SPDLOG_PERF_INFO(logger, "custom class with operator<<: {}..", obj);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto dur = end - begin;
    auto seconds = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
#if defined(PERF_LOG_MACRO_PRINT_DURATIONS)
    std::cout << "Log level requires writing using PERF runtime macro:" << seconds << "us" << std::endl;
#endif
  }
}
TEST_CASE("performace test when writing", "[PERF_log_macro]")
{
  MyObj obj;
  auto null_sink = std::make_shared<spdlog::sinks::null_sink_st>();
  auto logger = std::make_shared<spdlog::logger>("null_logger", null_sink);
  logger->set_level(spdlog::level::info);

  std::size_t loop_count = 1000;
  {
    auto begin = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < loop_count; i++)
    {
      logger->info("custom class with operator<<: {}..", obj);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto dur = end - begin;
    auto seconds = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
#if defined(PERF_LOG_MACRO_PRINT_DURATIONS)
    std::cout << "Log level requires writing using NORMAL interface: " << seconds << "us" << std::endl;
#endif
  }
}


TEST_CASE("performace test not writing", "[PERF_log_macro]")
{
  MyObj obj;
  auto null_sink = std::make_shared<spdlog::sinks::null_sink_st>();
  auto logger = std::make_shared<spdlog::logger>("null_logger", null_sink);
  logger->set_level(spdlog::level::info);

  std::size_t loop_count = 10000;
  {
    auto begin = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < loop_count; i++)
    {
      SPDLOG_PERF_TRACE(logger, "custom class with operator<<: {}..{}", obj, 1234);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto dur = end - begin;
    auto seconds = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
#if defined(PERF_LOG_MACRO_PRINT_DURATIONS)
    std::cout << "Log level does NOT require writing using PERF runtime macro: " << seconds << "us" << std::endl;
#endif
  }
  {
    auto begin = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < loop_count; i++)
    {
      logger->trace("custom class with operator<<: {}..{}", obj, 1234);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto dur = end - begin;
    auto seconds = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
#if defined(PERF_LOG_MACRO_PRINT_DURATIONS)
    std::cout << "Log level does NOT require writing using NORMAL interface: " << seconds << "us" << std::endl;
#endif
  }
}






