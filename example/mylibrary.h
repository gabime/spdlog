//
// Copyright(c) 2021
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifdef _WIN32
#define LIB_EXPORT __declspec(dllexport)
#else // !defined(_WIN32)
#define LIB_EXPORT
#endif

#include <memory>
#include <spdlog/logger.h>

namespace lib
{
    LIB_EXPORT void set_logger(const std::shared_ptr<spdlog::logger>& logger);

    LIB_EXPORT void test();
}
