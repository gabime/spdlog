// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/common.h>

#include <memory>
#include <vector>
#include <unordered_map>

namespace spdlog {
namespace sinks {

// 基于日志级别的sink，根据日志级别将日志分发到不同的sink
class SPDLOG_API level_based_sink : public sink {
public:
    explicit level_based_sink() = default;
    ~level_based_sink() override = default;

    level_based_sink(const level_based_sink &) = delete;
    level_based_sink &operator=(const level_based_sink &) = delete;

    // 添加sink到指定级别
    void add_sink_for_level(level::level_enum level, sink_ptr sink);
    
    // 添加sink到多个级别
    void add_sink_for_levels(std::initializer_list<level::level_enum> levels, sink_ptr sink);
    
    // 添加sink到所有级别
    void add_sink_for_all_levels(sink_ptr sink);
    
    // 移除指定级别上的指定sink
    void remove_sink_for_level(level::level_enum level, sink_ptr sink);
    
    // 移除指定级别上的所有sink
    void remove_all_sinks_for_level(level::level_enum level);
    
    // 移除所有sink
    void remove_all_sinks();
    
    // 设置sink的格式化器
    void set_formatter(std::unique_ptr<formatter> formatter) override;
    
    // 设置sink的日志级别
    void set_level(level::level_enum level) override;
    
    // 获取sink的日志级别
    level::level_enum level() const override;
    
    // 检查sink是否应该记录指定级别的日志
    bool should_log(level::level_enum level) const override;
    
    // 记录日志
    void log(const details::log_msg &msg) override;
    
    // 刷新sink
    void flush() override;
    
private:
    std::unordered_map<level::level_enum, std::vector<sink_ptr>> sinks_;
    std::vector<sink_ptr> all_level_sinks_;
    level::level_enum level_ = level::trace;
};

} // namespace sinks
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "level_based_sink-inl.h"
#endif