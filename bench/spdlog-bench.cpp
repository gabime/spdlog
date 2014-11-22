
#include "spdlog/spdlog.h"


int main(int, char* [])
{
    int howmany = 1000000;
    namespace spd = spdlog;
    ///Create a file rotating logger with 5mb size max and 3 rotated files
    auto logger = spd::rotating_logger_mt("file_logger", "logs/spd-sample", 100 *1024 * 1024 , 5);

    logger->set_pattern("[%Y-%b-%d %T.%e]: %v");
    for(int i  = 0 ; i < howmany; ++i)
        logger->info() << "spdlogger message #" << i;

    return 0;
}
