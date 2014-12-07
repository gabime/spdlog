
#include "spdlog/spdlog.h"


int main(int, char* [])
{
    int howmany = 1048576;
    namespace spd = spdlog;
    spd::set_async_mode(howmany);
    ///Create a file rotating logger with 5mb size max and 3 rotated files
    auto logger = spd::rotating_logger_mt("file_logger", "logs/spd-sample", 10 *1024 * 1024 , 5);

    logger->set_pattern("[%Y-%b-%d %T.%e]: %v");
    for(int i  = 0 ; i < howmany; ++i)
        logger->info() << "spdlog message #" << i << ": This is some text for your pleasure";


    //because spdlog async logger waits for the back thread logger to finish all messages upon destrcuting,
    //and we want to measure only the time it took to push those messages to the backthread..
    abort();
    return 0;
}
