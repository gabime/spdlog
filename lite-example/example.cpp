#include "logger.h"

int main()
{
    //auto l = spdlog::create_lite();
    spdlog::lite::info("HELLO info {}", 123);
}