#pragma once

#include "catch.hpp"
#include "utils.h"
#include <chrono>
#include <cstdio>
#include <exception>
#include <fstream>
#include <ostream>
#include <string>

#define SPDLOG_TRACE_ON
#define SPDLOG_DEBUG_ON

#include "../include/spdlog/sinks/null_sink.h"
#include "../include/spdlog/sinks/ostream_sink.h"
#include "../include/spdlog/spdlog.h"
