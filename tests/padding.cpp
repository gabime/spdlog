
#include "includes.h"

#ifdef SPDLOG_ENABLE_PATTERN_PADDING
template<class T>
std::string log_formatted(const T& what, const std::string& pattern = "%v")
{

  std::ostringstream oss;
  auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

  spdlog::logger oss_logger("oss", oss_sink);
  oss_logger.set_level(spdlog::level::info);
  oss_logger.set_pattern(pattern);
  oss_logger.info(what);

  return oss.str().substr(0, oss.str().length() - spdlog::details::os::eol_size);
}

TEST_CASE("padding", "[padding]")
{
  REQUIRE(log_formatted("Hello") == "Hello");
  REQUIRE(log_formatted("Hello", "%l %v") == "info Hello");
  REQUIRE(log_formatted("Hello", "%6l %v") == "  info Hello");
  REQUIRE(log_formatted("Hello", "%-6l %v") == "info   Hello");
  REQUIRE(log_formatted("Hello", "%06l %v") == "00info Hello");
  REQUIRE(log_formatted("Hello", "%-06l %v") == "info00 Hello");
  REQUIRE(log_formatted("Hello", "%12l %v") == "        info Hello");
  REQUIRE(log_formatted("Hello", "%-12l %v") == "info         Hello");
  REQUIRE(log_formatted("Hello", "%012l %v") == "00000000info Hello");
  REQUIRE(log_formatted("Hello", "%012l %v") == "00000000info Hello");
  REQUIRE(log_formatted("Hello", "%-012l %v") == "info00000000 Hello");
}
#endif
