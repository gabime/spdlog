// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/log_aggregator.h>
#endif

#include <spdlog/details/os.h>
#include <spdlog/fmt/fmt.h>
#include <regex>
#include <chrono>
#include <algorithm>
#include <iterator>

namespace spdlog {

log_aggregator::log_aggregator(size_t max_buffer_size)
    : max_buffer_size_(max_buffer_size)
    , log_callback_([this](const details::log_msg &msg) {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        if (log_buffer_.size() >= max_buffer_size_) {
            log_buffer_.erase(log_buffer_.begin());
        }
        log_buffer_.push_back(msg);
        update_statistics(msg);
        update_latency_distribution(msg);
    })
{
    // 初始化统计信息
    auto now = std::chrono::system_clock::now();
    statistics_.first_log_time = now;
    statistics_.last_log_time = now;
}

log_aggregator::~log_aggregator() {
    // 注销所有日志记录器
    std::lock_guard<std::mutex> lock(loggers_mutex_);
    for (auto &[name, logger] : registered_loggers_) {
        logger->remove_callback(log_callback_);
    }
}

void log_aggregator::register_logger(std::shared_ptr<logger> logger) {
    std::lock_guard<std::mutex> lock(loggers_mutex_);
    auto name = logger->name();
    if (registered_loggers_.find(name) == registered_loggers_.end()) {
        registered_loggers_[name] = logger;
        logger->add_callback(log_callback_);
    }
}

void log_aggregator::unregister_logger(std::shared_ptr<logger> logger) {
    std::lock_guard<std::mutex> lock(loggers_mutex_);
    auto name = logger->name();
    auto it = registered_loggers_.find(name);
    if (it != registered_loggers_.end()) {
        logger->remove_callback(log_callback_);
        registered_loggers_.erase(it);
    }
}

query_result log_aggregator::query_logs(const query_filter &filter) const {
    auto start_time = std::chrono::high_resolution_clock::now();
    query_result result;
    std::vector<details::log_msg> filtered_logs;

    std::lock_guard<std::mutex> lock(buffer_mutex_);
    result.total_logs = log_buffer_.size();

    // 过滤日志
    for (const auto &msg : log_buffer_) {
        if (msg.time < filter.start_time || msg.time > filter.end_time) {
            continue;
        }
        if (msg.level < filter.min_level || msg.level > filter.max_level) {
            continue;
        }
        if (!filter.logger_name.empty() && msg.logger_name != filter.logger_name) {
            continue;
        }
        if (!filter.keyword.empty()) {
            std::string payload(msg.payload.data(), msg.payload.size());
            if (payload.find(filter.keyword) == std::string::npos) {
                continue;
            }
        }
        filtered_logs.push_back(msg);
    }

    result.total_matched = filtered_logs.size();

    // 应用分页
    size_t start_idx = filter.offset;
    size_t end_idx = std::min(start_idx + filter.limit, filtered_logs.size());
    if (start_idx < filtered_logs.size()) {
        result.logs.insert(result.logs.end(), filtered_logs.begin() + start_idx, filtered_logs.begin() + end_idx);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    result.query_duration = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return result;
}

log_statistics log_aggregator::get_statistics() const {
    std::lock_guard<std::mutex> lock(statistics_mutex_);
    return statistics_;
}

latency_distribution log_aggregator::get_latency_distribution(const std::string &prefix) const {
    std::lock_guard<std::mutex> lock(latency_mutex_);
    auto it = latency_distributions_.find(prefix);
    if (it != latency_distributions_.end()) {
        return it->second;
    }
    return latency_distribution{};
}

void log_aggregator::clean_old_logs(std::chrono::system_clock::time_point before_time) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    auto it = std::remove_if(log_buffer_.begin(), log_buffer_.end(), 
        [&before_time](const details::log_msg &msg) {
            return msg.time < before_time;
        });
    log_buffer_.erase(it, log_buffer_.end());
}

void log_aggregator::set_max_buffer_size(size_t max_size) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    max_buffer_size_ = max_size;
    // 如果当前缓存大小超过新的最大大小，删除旧的日志
    if (log_buffer_.size() > max_size) {
        size_t to_remove = log_buffer_.size() - max_size;
        log_buffer_.erase(log_buffer_.begin(), log_buffer_.begin() + to_remove);
    }
}

size_t log_aggregator::get_current_buffer_size() const {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    return log_buffer_.size();
}

void log_aggregator::add_custom_statistic(const std::string &name, 
    std::function<void(const details::log_msg &, log_statistics &)> statistic_fn) {
    std::lock_guard<std::mutex> lock(custom_stats_mutex_);
    custom_statistics_[name] = std::move(statistic_fn);
}

std::unordered_map<std::string, double> log_aggregator::get_custom_statistics() const {
    std::lock_guard<std::mutex> lock(custom_stats_mutex_);
    std::unordered_map<std::string, double> result;
    for (const auto &[name, fn] : custom_statistics_) {
        // 这里可以根据需要实现自定义统计结果的获取
        // 目前只是返回0.0作为示例
        result[name] = 0.0;
    }
    return result;
}

void log_aggregator::update_statistics(const details::log_msg &msg) {
    std::lock_guard<std::mutex> lock(statistics_mutex_);
    
    statistics_.total_logs++;
    statistics_.last_log_time = msg.time;
    
    // 更新级别计数
    statistics_.level_counts[msg.level]++;
    
    // 调用自定义统计函数
    std::lock_guard<std::mutex> custom_lock(custom_stats_mutex_);
    for (auto &[name, fn] : custom_statistics_) {
        try {
            fn(msg, statistics_);
        } catch (const std::exception &ex) {
            // 忽略自定义统计函数的异常
        }
    }
}

void log_aggregator::update_latency_distribution(const details::log_msg &msg) {
    std::string payload(msg.payload.data(), msg.payload.size());
    std::optional<double> latency = parse_latency(payload);
    if (!latency) {
        return;
    }
    
    // 提取延迟前缀（如果有的话）
    std::string prefix;
    size_t colon_pos = payload.find(":");
    if (colon_pos != std::string::npos) {
        prefix = payload.substr(0, colon_pos);
    }
    
    std::lock_guard<std::mutex> lock(latency_mutex_);
    auto &dist = latency_distributions_[prefix];
    
    dist.count++;
    dist.min_latency = std::min(dist.min_latency, *latency);
    dist.max_latency = std::max(dist.max_latency, *latency);
    dist.average_latency = (dist.average_latency * (dist.count - 1) + *latency) / dist.count;
    
    // 将延迟分到不同的桶中
    std::string bucket;
    if (*latency < 1.0) {
        bucket = "<1ms";
    } else if (*latency < 10.0) {
        bucket = "1-10ms";
    } else if (*latency < 100.0) {
        bucket = "10-100ms";
    } else if (*latency < 1000.0) {
        bucket = "100-1000ms";
    } else {
        bucket = ">1000ms";
    }
    dist.latency_buckets[bucket]++;
}

std::optional<double> log_aggregator::parse_latency(const std::string &msg) const {
    // 尝试从日志消息中解析延迟信息
    // 支持的格式：
    // - latency: 123.45ms
    // - time: 123.45ms
    // - took: 123.45ms
    
    std::regex latency_regex(R"((latency|time|took):\s*(\d+(?:\.\d+)?)ms)"s);
    std::smatch match;
    if (std::regex_search(msg, match, latency_regex)) {
        try {
            double latency = std::stod(match[2]);
            return latency;
        } catch (const std::exception &ex) {
            // 解析失败
        }
    }
    return std::nullopt;
}

} // namespace spdlog