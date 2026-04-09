#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <memory>
#include <string>
#include <vector>

namespace module_network {

class NetworkLogger {
public:
    static NetworkLogger& instance();
    
    void init(const spdlog::filename_t& log_dir);
    void shutdown();
    
    std::shared_ptr<spdlog::logger> get_logger() const;
    spdlog::filename_t get_log_file() const;
    
    void log_connection(const std::string& host, int port);
    void log_disconnection(const std::string& host, int port);
    void log_data_sent(const std::string& host, size_t bytes);
    void log_data_received(const std::string& host, size_t bytes);
    void log_error(const std::string& error_msg);
    
    void set_level(spdlog::level::level_enum level);
    spdlog::level::level_enum get_level() const;
    
    void flush();

private:
    NetworkLogger() = default;
    ~NetworkLogger() = default;
    NetworkLogger(const NetworkLogger&) = delete;
    NetworkLogger& operator=(const NetworkLogger&) = delete;
    
    std::shared_ptr<spdlog::logger> logger_;
    spdlog::filename_t log_file_;
    bool initialized_ = false;
};

class NetworkSimulator {
public:
    NetworkSimulator();
    ~NetworkSimulator() = default;
    
    void connect(const std::string& host, int port);
    void disconnect(const std::string& host, int port);
    void send_data(const std::string& host, const std::string& data);
    std::string receive_data(const std::string& host, size_t max_bytes);
    
    size_t get_connections_count() const;
    void simulate_error(const std::string& error_msg);

private:
    std::vector<std::pair<std::string, int>> connections_;
};

void test_network_operations();
int get_module_id();

}
