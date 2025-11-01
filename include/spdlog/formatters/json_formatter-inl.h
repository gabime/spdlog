// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/formatters/json_formatter.h>
#endif

#include <spdlog/details/os.h>
#include <spdlog/details/format_helper.h>

namespace spdlog {
namespace formatters {

json_formatter::json_formatter(std::string eol)
    : eol_(std::move(eol))
{}

std::unique_ptr<formatter> json_formatter::clone() const {
    return details::make_unique<json_formatter>(*this);
}

void json_formatter::format(const details::log_msg &msg, memory_buf_t &dest) {
    fmt::format_to(std::back_inserter(dest), "{{");
    
    // 添加时间字段
    std::tm tm_time;
    if (msg.time.time_since_epoch().count() == 0) {
        // 如果时间未设置，使用当前时间
        auto now = std::chrono::system_clock::now();
        tm_time = details::os::localtime(std::chrono::system_clock::to_time_t(now));
    } else {
        tm_time = details::os::localtime(std::chrono::system_clock::to_time_t(msg.time));
    }
    
    char time_buf[128];
    std::strftime(time_buf, sizeof(time_buf), time_format_.c_str(), &tm_time);
    fmt::format_to(std::back_inserter(dest), "\"time\":\"{}\",", time_buf);
    
    // 添加日志级别
    fmt::format_to(std::back_inserter(dest), "\"level\":{},", static_cast<int>(msg.level));
    
    // 添加日志级别名称
    if (include_level_name_) {
        fmt::format_to(std::back_inserter(dest), "\"level_name\":\"{}\",", level::to_string_view(msg.level));
    }
    
    // 添加日志名称
    fmt::format_to(std::back_inserter(dest), "\"logger_name\":\"{}\",", msg.logger_name);
    
    // 添加线程ID
    if (include_thread_id_) {
        fmt::format_to(std::back_inserter(dest), "\"thread_id\":{},", msg.thread_id);
    }
    
    // 添加日志消息
    fmt::format_to(std::back_inserter(dest), "\"message\":{}", fmt::format("{}", msg.payload));
    
    // 添加源位置信息
    if (include_source_location_ && msg.source.filename != nullptr) {
        fmt::format_to(std::back_inserter(dest), ",\"source_location\":{{\"file\":\"{}\",\"line\":{},\"function\":\"{}\"}}",
            msg.source.filename,
            msg.source.line,
            msg.source.funcname);
    }
    
    // 关闭JSON对象
    fmt::format_to(std::back_inserter(dest), "{}", eol_);
}

void json_formatter::set_include_source_location(bool include) {
    include_source_location_ = include;
}

void json_formatter::set_include_thread_id(bool include) {
    include_thread_id_ = include;
}

void json_formatter::set_include_level_name(bool include) {
    include_level_name_ = include;
}

void json_formatter::set_time_format(std::string format) {
    time_format_ = std::move(format);
}

} // namespace formatters
} // namespace spdlog