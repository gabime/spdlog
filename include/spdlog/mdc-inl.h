// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/mdc.h>
#endif

SPDLOG_NAMESPACE_BEGIN

SPDLOG_INLINE mdc::mdc_map_t& mdc::get_context() {
    static thread_local mdc_map_t context;
    context_created() = true;
    return context;
}

SPDLOG_INLINE bool& mdc::context_created() noexcept {
    static thread_local bool created = false;
    return created;
}

SPDLOG_NAMESPACE_END
