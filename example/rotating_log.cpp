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
        // Create a file rotating logger with 5mb size max and 3 rotated files
        auto rotating_logger = spd::rotating_logger_mt("some_logger_name", "logs/rotating.txt", 1048576 * 5, 3);
        for (int i = 0; i < 10; ++i)
        {
            rotating_logger->info("{} * {} equals {:>10}", i, i, i * i);
        }
    }
    // Exceptions will only be thrown upon failed logger or sink construction (not during logging)
    catch (const spd::spdlog_ex &ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
        return 1;
    }
	return 0;
}
