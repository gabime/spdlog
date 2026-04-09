#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <memory>
#include <string>

namespace module_core {

class CoreLogger {
public:
    static CoreLogger& instance();
    
    void init(const std::string& log_dir);
    void shutdown();
    
    std::shared_ptr<spdlog::logger> get_logger() const;
    std::string get_log_file() const;
    
    void log_info(const std::string& message);
    void log_warn(const std::string& message);
    void log_error(const std::string& message);
    void log_debug(const std::string& message);
    
    void set_level(spdlog::level::level_enum level);
    spdlog::level::level_enum get_level() const;
    
    void flush();

private:
    CoreLogger() = default;
    ~CoreLogger() = default;
    CoreLogger(const CoreLogger&) = delete;
    CoreLogger& operator=(const CoreLogger&) = delete;
    
    std::shared_ptr<spdlog::logger> logger_;
    std::string log_file_;
    bool initialized_ = false;
};

void test_function();
int get_module_id();

}
