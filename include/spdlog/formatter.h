//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <spdlog/details/log_msg.h>

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

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

typedef std::function<void(details::log_msg& msg)> custom_flag_formatter;

class pattern_formatter : public formatter
{
public:
    explicit pattern_formatter(const std::string& pattern);
    pattern_formatter(const pattern_formatter&) = delete;
    pattern_formatter& operator=(const pattern_formatter&) = delete;
    void format(details::log_msg& msg) override;
    void add_customer_formatter(char sig, custom_flag_formatter formatter);

private:
    const std::string _pattern;
    std::unordered_map<char, custom_flag_formatter> _custom_formatters;
    std::vector<std::unique_ptr<details::flag_formatter>> _formatters;
    void handle_flag(char flag);
    void compile_pattern(const std::string& pattern);
};
}

#include <spdlog/details/pattern_formatter_impl.h>

