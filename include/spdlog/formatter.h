//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "details/log_msg.h"

#include <vector>
#include <string>
#include <memory>

namespace spdlog
{
namespace details
{
class flag_formatter;
}

class formatter
{
public:
    virtual ~formatter() {}
    virtual void format(details::log_msg& msg) = 0;
};

class pattern_formatter SPDLOG_FINAL : public formatter
{

public:
    explicit pattern_formatter(const std::string& pattern, pattern_time_type pattern_time = pattern_time_type::local);
    pattern_formatter(const pattern_formatter&) = delete;
    pattern_formatter& operator=(const pattern_formatter&) = delete;
    void format(details::log_msg& msg) override;
private:
    const std::string _pattern;
    const pattern_time_type _pattern_time;
    std::vector<std::unique_ptr<details::flag_formatter>> _formatters;
    std::tm get_time(details::log_msg& msg);
    void handle_flag(char flag);
    void compile_pattern(const std::string& pattern);
};
}

#include "details/pattern_formatter_impl.h"

