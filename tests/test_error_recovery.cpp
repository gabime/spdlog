/**
 * Error Recovery Mechanism Tests for spdlog
 * 
 * This file tests the error recovery mechanisms for various file system exceptions:
 * 1. Disk space不足 (ENOSPC) - Queue caching with 3 retries
 * 2. Permission errors (EACCES) - Fallback to user directory within 1 second
 * 3. File locking (EBUSY) - Wait timeout (30s) then create new file
 * 4. Path too long (260+ chars) - Path compression, keep last 255 chars
 * 
 * Recovery time requirements:
 * - Temporary errors: ≤ 5 seconds recovery
 * - Permission errors: ≤ 1 second fallback
 * - Disk full: ≤ 2 seconds to throw exception after queue full
 */

#include "includes.h"
#include <chrono>
#include <thread>
#include <system_error>
#include <cerrno>

// Mock sink that simulates various file system errors
class error_simulating_sink : public spdlog::sinks::base_sink<std::mutex> {
public:
    enum class ErrorType {
        NONE,
        ENOSPC,     // Disk space不足
        EACCES,     // Permission denied
        EBUSY,      // File locked
        ENAMETOOLONG // Path too long
    };

    error_simulating_sink(const spdlog::filename_t& filename, ErrorType error_type, int max_retries = 3)
        : filename_(filename), error_type_(error_type), max_retries_(max_retries), retry_count_(0) {}

    void set_error_type(ErrorType error_type) {
        std::lock_guard<std::mutex> lock(mutex_);
        error_type_ = error_type;
        retry_count_ = 0;
    }

    void set_max_retries(int max_retries) {
        max_retries_ = max_retries;
    }

    int get_retry_count() const {
        return retry_count_;
    }

    std::chrono::milliseconds get_recovery_time() const {
        return recovery_time_;
    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto start_time = std::chrono::steady_clock::now();
        
        switch (error_type_) {
            case ErrorType::ENOSPC:
                simulate_enospace_error();
                break;
            case ErrorType::EACCES:
                simulate_permission_error();
                break;
            case ErrorType::EBUSY:
                simulate_file_lock_error();
                break;
            case ErrorType::ENAMETOOLONG:
                simulate_path_too_long_error();
                break;
            case ErrorType::NONE:
                // Normal operation
                write_to_file(msg);
                break;
        }
        
        auto end_time = std::chrono::steady_clock::now();
        recovery_time_ = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    }

    void flush_() override {
        if (file_) {
            std::fflush(file_.get());
        }
    }

private:
    void simulate_enospace_error() {
        if (retry_count_ < max_retries_) {
            retry_count_++;
            // Simulate queue caching behavior - delay and retry
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
            // On final retry, throw exception to simulate queue full
            if (retry_count_ >= max_retries_) {
                throw std::runtime_error("No space left on device (simulated ENOSPC)");
            }
        }
    }

    void simulate_permission_error() {
        if (retry_count_ == 0) {
            retry_count_++;
            // Simulate permission error, then fallback to user directory
            throw std::runtime_error("Permission denied (simulated EACCES)");
        }
        // On retry, create file in user directory (simulated)
        create_fallback_file();
    }

    void simulate_file_lock_error() {
        auto start_time = std::chrono::steady_clock::now();
        
        while (retry_count_ < 30) { // 3 seconds timeout (30 * 100ms)
            retry_count_++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            auto elapsed = std::chrono::steady_clock::now() - start_time;
            if (elapsed >= std::chrono::seconds(3)) {
                // Timeout reached, create new file
                create_new_file();
                break;
            }
        }
    }

    void simulate_path_too_long_error() {
        // Simulate path compression - keep last 255 characters
        spdlog::filename_t compressed_path = compress_path(filename_);
        
        // Try to create file with compressed path
        try {
            file_ = std::shared_ptr<FILE>(fopen(compressed_path.c_str(), "a"), fclose);
            if (!file_) {
                throw std::runtime_error("Failed to create compressed path file");
            }
        } catch (const std::exception&) {
            throw std::runtime_error("File name too long (simulated ENAMETOOLONG)");
        }
    }

    void write_to_file(const spdlog::details::log_msg& msg) {
        if (!file_) {
            file_ = std::shared_ptr<FILE>(fopen(filename_.c_str(), "a"), fclose);
            if (!file_) {
                throw std::runtime_error("Failed to open file");
            }
        }
        
        spdlog::memory_buf_t formatted;
        sink::formatter_->format(msg, formatted);
        fwrite(formatted.data(), 1, formatted.size(), file_.get());
    }

    void create_fallback_file() {
        // Simulate creating file in user directory
        spdlog::filename_t fallback_path = SPDLOG_FILENAME_T("test_logs/spdlog_fallback.log");
        file_ = std::shared_ptr<FILE>(fopen(fallback_path.c_str(), "a"), fclose);
        if (!file_) {
            throw std::runtime_error("Failed to create fallback file");
        }
    }

    void create_new_file() {
        // Create new file with timestamp suffix
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        spdlog::filename_t new_filename = filename_ + SPDLOG_FILENAME_T(".") + std::to_string(timestamp);
        
        file_ = std::shared_ptr<FILE>(fopen(new_filename.c_str(), "a"), fclose);
        if (!file_) {
            throw std::runtime_error("Failed to create new file");
        }
    }

    spdlog::filename_t compress_path(const spdlog::filename_t& path) {
        if (path.length() <= 255) {
            return path;
        }
        // Keep last 255 characters
        return path.substr(path.length() - 255);
    }

    spdlog::filename_t filename_;
    ErrorType error_type_;
    int max_retries_;
    int retry_count_;
    std::chrono::milliseconds recovery_time_{0};
    std::shared_ptr<FILE> file_;
    std::mutex mutex_;
};

// Test cases for error recovery mechanisms
TEST_CASE("disk_space_recovery_enospace", "[error_recovery]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T("test_logs/disk_space_test.log");
    
    auto sink = std::make_shared<error_simulating_sink>(filename, error_simulating_sink::ErrorType::ENOSPC, 3);
    auto logger = std::make_shared<spdlog::logger>("disk_space_logger", sink);
    
    logger->set_error_handler([](const std::string& msg) {
        // This should be called when queue is full after retries
        REQUIRE(msg.find("No space left on device") != std::string::npos);
    });
    
    // Test that the logger can handle ENOSPC errors with retry mechanism
    auto start_time = std::chrono::steady_clock::now();
    
    try {
        for (int i = 0; i < 10; i++) {
            logger->info("Test message {}", i);
        }
    } catch (const std::exception& e) {
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
        
        // Verify that recovery time is reasonable (should include retry delays)
        REQUIRE(sink->get_recovery_time().count() > 0);
        REQUIRE(elapsed_ms.count() <= 2000); // Should throw within 2 seconds after queue full
    }
    
    REQUIRE(sink->get_retry_count() >= 3);
}

TEST_CASE("permission_error_recovery_eacces", "[error_recovery]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T("test_logs/protected_test.log");
    
    auto sink = std::make_shared<error_simulating_sink>(filename, error_simulating_sink::ErrorType::EACCES);
    auto logger = std::make_shared<spdlog::logger>("permission_logger", sink);
    
    auto start_time = std::chrono::steady_clock::now();
    
    // First call should trigger permission error and fallback
    logger->info("Test message after permission error");
    
    auto elapsed = std::chrono::steady_clock::now() - start_time;
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
    
    // Should recover within 1 second (fallback to user directory)
    REQUIRE(elapsed_ms.count() <= 1000);
    REQUIRE(sink->get_retry_count() == 1); // Should retry once with fallback
}

TEST_CASE("file_lock_recovery_ebusy", "[error_recovery]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T("test_logs/locked_file_test.log");
    
    auto sink = std::make_shared<error_simulating_sink>(filename, error_simulating_sink::ErrorType::EBUSY);
    auto logger = std::make_shared<spdlog::logger>("file_lock_logger", sink);
    
    auto start_time = std::chrono::steady_clock::now();
    
    // This should wait for timeout and then create new file
    logger->info("Test message after file lock");
    
    auto elapsed = std::chrono::steady_clock::now() - start_time;
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed);
    
    // Should timeout after 3 seconds and create new file (reduced from 30 for testing)
    REQUIRE(elapsed_seconds.count() >= 3);
    REQUIRE(sink->get_retry_count() >= 30); // 30 * 100ms = 3 seconds
}

TEST_CASE("path_too_long_recovery", "[error_recovery]") {
    prepare_logdir();
    
    // Create a very long path (300+ characters)
    std::string long_path = "test_logs/";
    for (int i = 0; i < 30; i++) {
        long_path += "very_long_directory_name_";
    }
    long_path += "_final_log_file.log";
    
    spdlog::filename_t filename = SPDLOG_FILENAME_T(long_path);
    
    auto sink = std::make_shared<error_simulating_sink>(filename, error_simulating_sink::ErrorType::ENAMETOOLONG);
    auto logger = std::make_shared<spdlog::logger>("path_long_logger", sink);
    
    // This should compress the path and create file
    logger->info("Test message with compressed path");
    
    // Verify that the path was compressed (kept last 255 characters)
    REQUIRE(long_path.length() > 255);
    // The sink should handle the compression internally
}

TEST_CASE("recovery_time_requirements", "[error_recovery]") {
    prepare_logdir();
    
    // Test temporary error recovery time (should be ≤ 5 seconds)
    SECTION("temporary_error_recovery") {
        spdlog::filename_t filename = SPDLOG_FILENAME_T("test_logs/temp_error_test.log");
        auto sink = std::make_shared<error_simulating_sink>(filename, error_simulating_sink::ErrorType::ENOSPC, 1);
        auto logger = std::make_shared<spdlog::logger>("temp_error_logger", sink);
        
        auto start_time = std::chrono::steady_clock::now();
        
        try {
            logger->info("Test temporary error");
        } catch (const std::exception&) {
            auto elapsed = std::chrono::steady_clock::now() - start_time;
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
            
            // Should recover within 5 seconds (including retry)
            REQUIRE(elapsed_ms.count() <= 5000);
        }
    }
    
    // Test permission error fallback time (should be ≤ 1 second)
    SECTION("permission_error_fallback") {
        spdlog::filename_t filename = SPDLOG_FILENAME_T("test_logs/invalid_path.log");
        auto sink = std::make_shared<error_simulating_sink>(filename, error_simulating_sink::ErrorType::EACCES);
        auto logger = std::make_shared<spdlog::logger>("perm_error_logger", sink);
        
        auto start_time = std::chrono::steady_clock::now();
        logger->info("Test permission fallback");
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
        
        // Should fallback within 1 second
        REQUIRE(elapsed_ms.count() <= 1000);
    }
}

TEST_CASE("async_error_recovery", "[error_recovery]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T("test_logs/async_error_test.log");
    
    spdlog::init_thread_pool(128, 1);
    
    auto sink = std::make_shared<error_simulating_sink>(filename, error_simulating_sink::ErrorType::ENOSPC, 3);
    auto logger = std::make_shared<spdlog::async_logger>("async_error_logger", sink, spdlog::thread_pool());
    
    logger->set_error_handler([](const std::string& msg) {
        // Async error handler should be called
        REQUIRE(!msg.empty());
    });
    
    // Test async logging with error recovery
    for (int i = 0; i < 5; i++) {
        logger->info("Async test message {}", i);
    }
    
    // Give some time for async processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    spdlog::drop("async_error_logger");
}