//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "fmt/fmt.h"
#include "spdlog/details/log_msg.h"

#include <memory>
#include <string>
#include <vector>

namespace spdlog {

class formatter
{
public:
    virtual ~formatter() = default;
    virtual void format(const details::log_msg &msg, fmt::memory_buffer &dest) = 0;
};

namespace details {
class flag_formatter;
}

class pattern_formatter SPDLOG_FINAL : public formatter
{
public:
    explicit pattern_formatter(const std::string &pattern, pattern_time_type pattern_time = pattern_time_type::local,
        std::string eol = spdlog::details::os::default_eol);
    pattern_formatter(const pattern_formatter &) = default;
    pattern_formatter &operator=(const pattern_formatter &) = default;
    void format(const details::log_msg &msg, fmt::memory_buffer &dest) override;

private:
    const std::string eol_;
    const pattern_time_type pattern_time_;
    std::vector<std::unique_ptr<details::flag_formatter>> formatters_;
    std::tm get_time(const details::log_msg &msg);
    void handle_flag(char flag);
    void compile_pattern(const std::string &pattern);
};
} // namespace spdlog

#include "details/pattern_formatter_impl.h"
