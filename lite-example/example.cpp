#define SPDLITE_ACTIVE_LEVEL SPDLITE_LEVEL_INFO
#include "logger.h"

int main()
{
    //auto l = spdlog::create_lite();
    //spdlog::lite::info("HELLO info {}", 123);
    SPDLITE_TRACE("SOME MACRO {}", 123);
    SPDLITE_INFO("SOME MACRO {}", "HHHHH");

}