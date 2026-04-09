#include "module_data.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <thread>
#include <random>

namespace module_data {

namespace {
const char* const LOGGER_NAME = "data_logger";
const int MODULE_ID = 3;
}

DataLogger& DataLogger::instance() {
    static DataLogger instance;
    return instance;
}

void DataLogger::init(const std::string& log_dir, size_t queue_size, size_t thread_count) {
    if (initialized_) {
        return;
    }
    
    log_file_ = log_dir + "/data.log";
    
    thread_pool_ = std::make_shared<spdlog::details::thread_pool>(queue_size, thread_count);
    
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_, true);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    
    std::vector<spdlog::sink_ptr> sinks = {file_sink, console_sink};
    logger_ = std::make_shared<spdlog::async_logger>(
        LOGGER_NAME, 
        sinks.begin(), 
        sinks.end(),
        thread_pool_,
        spdlog::async_overflow_policy::block
    );
    
    logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] %v");
    logger_->set_level(spdlog::level::debug);
    logger_->flush_on(spdlog::level::err);
    
    spdlog::register_logger(logger_);
    initialized_ = true;
}

void DataLogger::shutdown() {
    if (logger_) {
        logger_->flush();
        spdlog::drop(LOGGER_NAME);
        logger_.reset();
    }
    if (thread_pool_) {
        thread_pool_.reset();
    }
    initialized_ = false;
}

std::shared_ptr<spdlog::logger> DataLogger::get_logger() const {
    return logger_;
}

std::string DataLogger::get_log_file() const {
    return log_file_;
}

void DataLogger::log_processing_start(const std::string& dataset_name) {
    if (logger_) {
        logger_->info("[Data] Starting processing dataset: {}", dataset_name);
    }
}

void DataLogger::log_processing_end(const std::string& dataset_name,
                                     const std::chrono::milliseconds& duration,
                                     size_t records_processed) {
    if (logger_) {
        logger_->info("[Data] Finished processing dataset: {} in {} ms, records: {}",
                      dataset_name, duration.count(), records_processed);
    }
}

void DataLogger::log_record_processed(size_t record_id, const std::string& status) {
    if (logger_) {
        logger_->debug("[Data] Record {} processed: {}", record_id, status);
    }
}

void DataLogger::log_error(const std::string& error_msg, size_t record_id) {
    if (logger_) {
        if (record_id > 0) {
            logger_->error("[Data] Error processing record {}: {}", record_id, error_msg);
        } else {
            logger_->error("[Data] Error: {}", error_msg);
        }
    }
}

void DataLogger::log_warning(const std::string& warning_msg) {
    if (logger_) {
        logger_->warn("[Data] Warning: {}", warning_msg);
    }
}

void DataLogger::set_level(spdlog::level::level_enum level) {
    if (logger_) {
        logger_->set_level(level);
    }
}

spdlog::level::level_enum DataLogger::get_level() const {
    if (logger_) {
        return logger_->level();
    }
    return spdlog::level::off;
}

void DataLogger::flush() {
    if (logger_) {
        logger_->flush();
    }
}

size_t DataLogger::get_overrun_counter() const {
    if (thread_pool_) {
        return thread_pool_->overrun_counter();
    }
    return 0;
}

size_t DataLogger::get_discard_counter() const {
    if (thread_pool_) {
        return thread_pool_->discard_counter();
    }
    return 0;
}

DataProcessor::DataProcessor() = default;

void DataProcessor::load_dataset(const std::string& name, size_t record_count) {
    dataset_name_ = name;
    total_records_ = record_count;
    processed_records_ = 0;
    
    auto& logger = DataLogger::instance();
    logger.log_processing_start(name);
}

void DataProcessor::process_all_records() {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < total_records_; ++i) {
        process_record(i);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    auto& logger = DataLogger::instance();
    logger.log_processing_end(dataset_name_, duration, processed_records_);
}

void DataProcessor::process_record(size_t record_id) {
    auto& logger = DataLogger::instance();
    
    bool is_error = std::find(error_records_.begin(), error_records_.end(), record_id) != error_records_.end();
    bool is_warning = std::find(warning_records_.begin(), warning_records_.end(), record_id) != warning_records_.end();
    
    if (is_error) {
        logger.log_error("Processing failed", record_id);
    } else if (is_warning) {
        logger.log_warning("Record " + std::to_string(record_id) + " has anomalies");
        logger.log_record_processed(record_id, "warning");
        processed_records_++;
    } else {
        logger.log_record_processed(record_id, "success");
        processed_records_++;
    }
}

size_t DataProcessor::get_total_records() const {
    return total_records_;
}

size_t DataProcessor::get_processed_records() const {
    return processed_records_;
}

const std::string& DataProcessor::get_dataset_name() const {
    return dataset_name_;
}

void DataProcessor::simulate_error_on_record(size_t record_id) {
    error_records_.push_back(record_id);
}

void DataProcessor::simulate_warning_on_record(size_t record_id) {
    warning_records_.push_back(record_id);
}

void test_data_processing() {
    DataProcessor processor;
    processor.load_dataset("test_dataset", 100);
    processor.simulate_error_on_record(42);
    processor.simulate_warning_on_record(75);
    processor.process_all_records();
}

int get_module_id() {
    return MODULE_ID;
}

}
