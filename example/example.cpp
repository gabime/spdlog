#include "spdlog/spdlog.h"

int main()
{
    auto l = spdlog::default_logger();
    l->trace("HELLO {}!!!", "lite");
    l->debug("HELLO {}!!!", "lite");
    l->info("HELLO {}!!!", "lite");
    l->warn("HELLO {}!!!", "lite");
    l->critical("HELLO s{}!!!", "lite");
}