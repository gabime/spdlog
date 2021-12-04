//
// Copyright(c) 2021
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "mylibrary.h"

int main(int, char *[])
{
    auto new_logger = spdlog::basic_logger_mt("library_example", "logs/library_example.txt", true);
    spdlog::set_level(spdlog::level::info);

    spdlog::set_default_logger(new_logger);
    #ifndef SPDLOG_SHARED_LIB
    lib::set_logger(new_logger);
    #endif

    spdlog::info("This message is from the base application");
    lib::test();
}
