#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <memory>
#include <string>
#include <vector>
#include <chrono>

namespace module_data {

class DataLogger {
public:
    static DataLogger& instance();
    
    void init(const std::string& log_dir, size_t queue_size = 8192, size_t thread_count = 1);
    void shutdown();
    
    std::shared_ptr<spdlog::logger> get_logger() const;
    std::string get_log_file() const;
    
    void log_processing_start(const std::string& dataset_name);
    void log_processing_end(const std::string& dataset_name, 
                            const std::chrono::milliseconds& duration,
                            size_t records_processed);
    void log_record_processed(size_t record_id, const std::string& status);
    void log_error(const std::string& error_msg, size_t record_id = 0);
    void log_warning(const std::string& warning_msg);
    
    void set_level(spdlog::level::level_enum level);
    spdlog::level::level_enum get_level() const;
    
    void flush();
    size_t get_overrun_counter() const;
    size_t get_discard_counter() const;

private:
    DataLogger() = default;
    ~DataLogger() = default;
    DataLogger(const DataLogger&) = delete;
    DataLogger& operator=(const DataLogger&) = delete;
    
    std::shared_ptr<spdlog::logger> logger_;
    std::shared_ptr<spdlog::details::thread_pool> thread_pool_;
    std::string log_file_;
    bool initialized_ = false;
};

class DataProcessor {
public:
    DataProcessor();
    ~DataProcessor() = default;
    
    void load_dataset(const std::string& name, size_t record_count);
    void process_all_records();
    void process_record(size_t record_id);
    
    size_t get_total_records() const;
    size_t get_processed_records() const;
    const std::string& get_dataset_name() const;
    
    void simulate_error_on_record(size_t record_id);
    void simulate_warning_on_record(size_t record_id);

private:
    std::string dataset_name_;
    size_t total_records_ = 0;
    size_t processed_records_ = 0;
    std::vector<size_t> error_records_;
    std::vector<size_t> warning_records_;
};

void test_data_processing();
int get_module_id();

}
