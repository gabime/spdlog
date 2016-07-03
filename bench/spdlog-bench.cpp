//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "spdlog/spdlog.h"


int main(int, char* [])
{
    int howmany = 1000000;
    namespace spd = spdlog;
    ///Create a file rotating logger with 5mb size max and 3 rotated files
    auto logger = spdlog::create<spd::sinks::simple_file_sink_st>("file_logger", "logs/spd-bench-st.txt", false);

    logger->set_pattern("[%Y-%b-%d %T.%e]: %v");
    for(int i  = 0 ; i < howmany; ++i)
        logger->info("spdlog message #{} : This is some text for your pleasure", i);
    return 0;
}
