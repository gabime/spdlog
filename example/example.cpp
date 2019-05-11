//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
//
// spdlog usage example
//
//

//#include "spdlog/spdlog.h"
#include "spdlog/logger.h"
//#include "spdlog/sinks/stdout_color_sinks.h"
int main(int, char *[])
{
    spdlog::logger *l = nullptr;
    const int i = 123;
    l->info("HELLO STATIC! {}", i);
    l->info("HELLO STATIC! {}", "GABI");
    l->info("HELLO STATIC! {} {}", "GABI", i);
    l->warn("HELLO STATIC! {} {}", "GABI", i);
}