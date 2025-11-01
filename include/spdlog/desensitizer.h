// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/details/log_msg.h>
#include <spdlog/fmt/fmt.h>
#include <regex>
#include <string>
#include <unordered_map>

namespace spdlog {
namespace details {

struct desensitize_rule {
    std::regex pattern;
    std::string replacement;
};

using desensitize_rules = std::unordered_map<std::string, desensitize_rule>;

} // namespace details

class SPDLOG_API desensitizer {
public:
    desensitizer();
    explicit desensitizer(details::desensitize_rules rules);
    
    // 添加脱敏规则
    void add_rule(const std::string &name, const std::string &pattern, const std::string &replacement);
    
    // 移除脱敏规则
    void remove_rule(const std::string &name);
    
    // 清空所有脱敏规则
    void clear_rules();
    
    // 脱敏字符串
    std::string desensitize(const std::string &msg) const;
    
    // 脱敏log_msg
    void desensitize(details::log_msg &msg) const;
    
private:
    details::desensitize_rules rules_;
};

} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "desensitizer-inl.h"
#endif