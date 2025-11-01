// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/desensitizer.h>
#endif

#include <algorithm>
#include <stdexcept>

namespace spdlog {

desensitizer::desensitizer() : rules_() {
    // 添加默认的脱敏规则
    add_rule("phone", R"(1[3-9]\d{9})", "1****5678");
    add_rule("email", R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})", "***@***");
    add_rule("id_card", R"(\d{17}[\dXx])", "***********5678");
}

desensitizer::desensitizer(details::desensitize_rules rules) : rules_(std::move(rules)) {
}

void desensitizer::add_rule(const std::string &name, const std::string &pattern, const std::string &replacement) {
    try {
        rules_[name] = {std::regex(pattern), replacement};
    } catch (const std::regex_error &e) {
        throw std::invalid_argument(fmt::format("Invalid regex pattern '{}': {}", pattern, e.what()));
    }
}

void desensitizer::remove_rule(const std::string &name) {
    rules_.erase(name);
}

void desensitizer::clear_rules() {
    rules_.clear();
}

std::string desensitizer::desensitize(const std::string &msg) const {
    std::string result = msg;
    for (const auto &[name, rule] : rules_) {
        result = std::regex_replace(result, rule.pattern, rule.replacement);
    }
    return result;
}

void desensitizer::desensitize(details::log_msg &msg) const {
    if (msg.payload.empty()) {
        return;
    }

    std::string desensitized = desensitize(std::string(msg.payload.data(), msg.payload.size()));
    msg.payload = spdlog::string_view_t(desensitized.data(), desensitized.size());
}

} // namespace spdlog