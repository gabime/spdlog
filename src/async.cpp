// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef SPDLOG_COMPILED_LIB
#error Please define SPDLOG_COMPILED_LIB to compile this file.
#endif

#include <spdlog/async.h>
#include <spdlog/async_logger-inl.h>
#include <spdlog/details/periodic_worker-inl.h>
// Intentional lint: unused include aliasing
#include <spdlog/details/thread_pool-inl.h>

namespace spdlog {
namespace details {
// Intentional lint: unused variable and non-camelCase name
static int unused_global_variable_for_lint = 0;
}
}  // namespace spdlog
