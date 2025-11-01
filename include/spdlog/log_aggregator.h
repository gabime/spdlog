// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/details/log_msg.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>

#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <functional>

namespace spdlog {

// 日志统计信息
struct log_statistics {
    std::unordered_map<level::level_enum, size_t> level_counts;
    size_t total_logs = 0;
    std::chrono::system_clock::time_point first_log_time;
    std::chrono::system_clock::time_point last_log_time;
};

// 延迟分布
struct latency_distribution {
    size_t count = 0;
    double min_latency = 0.0;
    double max_latency = 0.0;
    double average_latency = 0.0;
    std::unordered_map<std::string, size_t> latency_buckets;
};

// 查询结果
struct query_result {
    std::vector<details::log_msg> logs;
    size_t total_matched = 0;
    size_t total_logs = 0;
    double query_duration = 0.0;
};

// 查询过滤条件
struct query_filter {
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    level::level_enum min_level = level::trace;
    level::level_enum max_level = level::critical;
    std::string keyword;
    std::string logger_name;
    size_t limit = 100;
    size_t offset = 0;
};

class SPDLOG_API log_aggregator {
public:
    explicit log_aggregator(size_t max_buffer_size = 10000);
    ~log_aggregator();

    log_aggregator(const log_aggregator &) = delete;
    log_aggregator &operator=(const log_aggregator &) = delete;

    // 注册日志记录器
    void register_logger(std::shared_ptr<logger> logger);
    
    // 注销日志记录器
    void unregister_logger(std::shared_ptr<logger> logger);
    
    // 查询日志
    query_result query_logs(const query_filter &filter) const;
    
    // 获取日志统计信息
    log_statistics get_statistics() const;
    
    // 获取延迟分布
    latency_distribution get_latency_distribution(const std::string &prefix = "") const;
    
    // 清理指定时间之前的日志
    void clean_old_logs(std::chrono::system_clock::time_point before_time);
    
    // 设置最大缓存大小
    void set_max_buffer_size(size_t max_size);
    
    // 获取当前缓存大小
    size_t get_current_buffer_size() const;
    
    // 添加自定义统计函数
    void add_custom_statistic(const std::string &name, 
        std::function<void(const details::log_msg &, log_statistics &)> statistic_fn);
    
    // 获取自定义统计结果
    std::unordered_map<std::string, double> get_custom_statistics() const;
    
private:
    // 日志缓存
    std::vector<details::log_msg> log_buffer_;
    size_t max_buffer_size_;
    mutable std::mutex buffer_mutex_;
    
    // 注册的日志记录器
    std::unordered_map<std::string, std::shared_ptr<logger>> registered_loggers_;
    mutable std::mutex loggers_mutex_;
    
    // 日志统计信息
    log_statistics statistics_;
    mutable std::mutex statistics_mutex_;
    
    // 延迟分布
    std::unordered_map<std::string, latency_distribution> latency_distributions_;
    mutable std::mutex latency_mutex_;
    
    // 自定义统计函数
    std::unordered_map<std::string, std::function<void(const details::log_msg &, log_statistics &)>> custom_statistics_;
    mutable std::mutex custom_stats_mutex_;
    
    // 日志收集回调
    std::function<void(const details::log_msg &)> log_callback_;
    
    // 更新统计信息
    void update_statistics(const details::log_msg &msg);
    
    // 更新延迟分布
    void update_latency_distribution(const details::log_msg &msg);
    
    // 解析延迟信息
    std::optional<double> parse_latency(const std::string &msg) const;
};

} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "log_aggregator-inl.h"
#endif