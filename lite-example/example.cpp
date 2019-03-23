#include "logger.h"

int main()
{
    auto l = spdlog::create_lite();
    l.trace("HELLO {}!!!", "lite");
}