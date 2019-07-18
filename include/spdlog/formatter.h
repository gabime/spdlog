// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/common.h"
#include "fmt/fmt.h"
#include "spdlog/details/log_msg.h"

namespace spdlog {

class SPDLOG_API formatter
{
public:
    virtual ~formatter() = default;
    virtual void format(const details::log_msg &msg, fmt::memory_buffer &dest) = 0;
    virtual std::unique_ptr<formatter> clone() const = 0;
};
} // namespace spdlog
