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
        // Create a daily logger - a new file is created every day on 2:30am
        auto daily_logger = spd::daily_logger_mt("daily_logger", "logs/daily.txt", 2, 30);
        // trigger flush if the log severity is error or higher
        daily_logger->flush_on(spd::level::err);
        daily_logger->info(123.44);
    }
    // Exceptions will only be thrown upon failed logger or sink construction (not during logging)
    catch (const spd::spdlog_ex &ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
        return 1;
    }
	return 0;
}
