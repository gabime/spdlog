//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
//
// spdlog usage example
//
//

#include "spdlog/spdlog.h"

#include <iostream>
#include <string>

namespace spd = spdlog;

int main(int, char *[])
{
    try
    {
        // Create basic file logger (not rotated)
        auto my_logger = spd::basic_logger_mt("basic_logger", "logs/basic-log.txt");
        my_logger->info("Some log message");
    }
    // Exceptions will only be thrown upon failed logger or sink construction (not during logging)
    catch (const spd::spdlog_ex &ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
        return 1;
    }
	return 0;
}
