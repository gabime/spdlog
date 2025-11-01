// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/formatter.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/fmt/fmt.h>

#include <string>
#include <vector>

namespace spdlog {
namespace formatters {

class SPDLOG_API json_formatter : public formatter {
public:
    json_formatter(std::string eol = spdlog::details::os::default_eol);
    
    std::unique_ptr<formatter> clone() const override;
    void format(const details::log_msg &msg, memory_buf_t &dest) override;
    
    // 设置是否包含源位置信息
    void set_include_source_location(bool include);
    
    // 设置是否包含线程ID
    void set_include_thread_id(bool include);
    
    // 设置是否包含日志级别名称
    void set_include_level_name(bool include);
    
    // 设置时间格式
    void set_time_format(std::string format);
    
private:
    std::string eol_;
    bool include_source_location_ = true;
    bool include_thread_id_ = true;
    bool include_level_name_ = true;
    std::string time_format_ = "%Y-%m-%dT%H:%M:%S.%eZ";
};

} // namespace formatters
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "json_formatter-inl.h"
#endif