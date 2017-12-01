#pragma once

#include <cstdio>
#include <fstream>
#include <string>
#include <ostream>
#include <chrono>
#include <exception>
// regex supported only from gcc 4.9 and above
#if !(__GNUC__ <= 4 && __GNUC_MINOR__ < 9)
#include <regex>
#endif
#include "catch.hpp"
#include "utils.h"

#define SPDLOG_TRACE_ON
#define SPDLOG_DEBUG_ON

#include "../include/spdlog/spdlog.h"
#include "../include/spdlog/sinks/null_sink.h"
#include "../include/spdlog/sinks/ostream_sink.h"

