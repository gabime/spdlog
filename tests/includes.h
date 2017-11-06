#pragma once

#include <cstdio>
#include <fstream>
#include <string>
#include <ostream>
#include <chrono>
#include <exception>

#include "catch.hpp"
#include "utils.h"

#define SPDLOG_TRACE_ON
#define SPDLOG_DEBUG_ON

#include "../include/spdlog/spdlog.h"
#include "../include/spdlog/sinks/null_sink.h"
#include "../include/spdlog/sinks/ostream_sink.h"

