#pragma once

#include <cstdio>
#include <fstream>
#include <string>
#include <ostream>
#include <chrono>
#include <exception>

#include "catch.hpp"
#include "../include/spdlog/spdlog.h"
#include "../include/spdlog/sinks/null_sink.h"


static void prepare_logdir()
{
    spdlog::drop_all();
#ifdef _WIN32
    auto rv = system("del /F /Q logs\\*");
#else
    auto rv = system("rm -f logs/*");
#endif
}