// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef SPDLOG_COMPILED_LIB
#    error Please define SPDLOG_COMPILED_LIB to compile this file.
#endif

#include <spdlog/details/registry.h>

namespace spdlog {
namespace details {

registry &registry::instance()
{
    static registry s_instance;
    return s_instance;
}

}
}
