//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "details/log_msg.h"

#include <memory>
#include <string>
#include <vector>

namespace spdlog {
namespace details {
class flag_formatter;
}

class formatter
{
public:
    virtual ~formatter() = default;
    virtual void format(details::log_msg &msg) = 0;
};

class pattern_formatter SPDLOG_FINAL : public formatter
{
public:
    explicit pattern_formatter(const string &pattern, pattern_time_type pattern_time = pattern_time_type::local,
        string eol = spdlog::details::os::default_eol);
    pattern_formatter(const pattern_formatter &) = delete;
    pattern_formatter &operator=(const pattern_formatter &) = delete;
    void format(details::log_msg &msg) override;

private:
    const string _eol;
    const string _pattern;
    const pattern_time_type _pattern_time;
    vector<unique_ptr<details::flag_formatter>> _formatters;
    std::tm get_time(details::log_msg &msg);
    void handle_flag(char flag);
    void compile_pattern(const string &pattern);
};
} // namespace spdlog

#include "details/pattern_formatter_impl.h"
