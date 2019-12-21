#include "includes.h"
#include "test_sink.h"

#include <spdlog/cfg/env.h>

TEST_CASE("env", "[cfg]")
{
    spdlog::logger l{"l1"};
}

