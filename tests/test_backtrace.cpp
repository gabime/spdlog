#include "includes.h"
#include "test_sink.h"




TEST_CASE("bactrace1", "[bactrace]")
{

	using spdlog::sinks::test_sink_st;	
	auto test_sink = std::make_shared<test_sink_st>();
	int backtrace_size = 5;

	spdlog::logger logger("test", test_sink);
	logger.set_pattern("%v");
	logger.enable_backtrace(backtrace_size);

	logger.info("info message");   
	for(int i = 0; i < 100; i++)
		logger.debug("debug message {}", i);
	
	REQUIRE(test_sink->lines().size() == 1);
	REQUIRE(test_sink->lines()[0] == "info message");

	logger.dump_backtrace();
	REQUIRE(test_sink->lines().size() == 8);	
	REQUIRE(test_sink->lines()[1] == "****************** Backtrace Start ******************");	
	
	for(int i = 0; i < backtrace_size; i++)
	{
		REQUIRE(test_sink->lines()[i+2] == fmt::format("debug message {}", 100 - backtrace_size + i));
	}
	REQUIRE(test_sink->lines()[7] == "****************** Backtrace End ********************");
		
}

