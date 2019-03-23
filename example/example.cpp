#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"

int main()
{
    // auto l = spdlog::create_lite();
    // spdlog::lite::info("HELLO info {}", 123);

    SPDLOG_TRACE("SOME MACRO {}", 123);
    SPDLOG_INFO("SOME MACRO {}", "HHHHH");
}