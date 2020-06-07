// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <spdlog/async.h>
template class SPDLOG_API spdlog::details::mpmc_blocking_queue<spdlog::details::async_msg>;