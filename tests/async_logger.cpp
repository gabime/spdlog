
#include <iostream>

#include "includes.h"
#include "../include/spdlog/common.h"
#include "../include/spdlog/tweakme.h"


TEST_CASE("async_logging_overflow ", "[async_logging]")
{
	std::string filename = "logs/async_log_overflow.txt";
	auto sink = std::make_shared<spdlog::sinks::simple_file_sink_st>(filename, true);
	auto logger = std::make_shared<spdlog::async_logger>(
		"overflow_logger", 
		sink,
		2, // queue size
		spdlog::async_overflow_policy::discard_log_msg
	);
	for (int i = 0; i < 8; i++) {
		logger->info("Message #{}", i);
	}
	logger->flush();
	logger.reset();
	std::string the_log = file_contents(filename);
#if defined(SPDLOG_ASYNC_COUNT_DISCARDED_MSG)
	std::cout << the_log << std::endl;
	REQUIRE(the_log.find("Dropped 6 messages") != std::string::npos);
#endif
}









