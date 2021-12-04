//
// Copyright(c) 2021
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "mylibrary.h"

#include <spdlog/spdlog.h>

namespace lib
{
    const std::string logger_name = "example";

    void set_logger(const std::shared_ptr<spdlog::logger>& logger)
    {
        spdlog::set_default_logger(logger);
    }

    void test()
    {
        spdlog::info("This message is from the shared library.");
    }
}