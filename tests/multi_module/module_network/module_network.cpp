#include "module_network.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <mutex>
#include <random>

namespace module_network {

namespace {
const char* const LOGGER_NAME = "network_logger";
const int MODULE_ID = 2;
}

NetworkLogger& NetworkLogger::instance() {
    static NetworkLogger instance;
    return instance;
}

void NetworkLogger::init(const std::string& log_dir) {
    if (initialized_) {
        return;
    }
    
    log_file_ = log_dir + "/network.log";
    
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_, true);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    
    std::vector<spdlog::sink_ptr> sinks = {file_sink, console_sink};
    logger_ = std::make_shared<spdlog::logger>(LOGGER_NAME, sinks.begin(), sinks.end());
    
    logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] %v");
    logger_->set_level(spdlog::level::info);
    logger_->flush_on(spdlog::level::err);
    
    spdlog::register_logger(logger_);
    initialized_ = true;
}

void NetworkLogger::shutdown() {
    if (logger_) {
        logger_->flush();
        spdlog::drop(LOGGER_NAME);
        logger_.reset();
    }
    initialized_ = false;
}

std::shared_ptr<spdlog::logger> NetworkLogger::get_logger() const {
    return logger_;
}

std::string NetworkLogger::get_log_file() const {
    return log_file_;
}

void NetworkLogger::log_connection(const std::string& host, int port) {
    if (logger_) {
        logger_->info("[Network] Connected to {}:{}", host, port);
    }
}

void NetworkLogger::log_disconnection(const std::string& host, int port) {
    if (logger_) {
        logger_->info("[Network] Disconnected from {}:{}", host, port);
    }
}

void NetworkLogger::log_data_sent(const std::string& host, size_t bytes) {
    if (logger_) {
        logger_->debug("[Network] Sent {} bytes to {}", bytes, host);
    }
}

void NetworkLogger::log_data_received(const std::string& host, size_t bytes) {
    if (logger_) {
        logger_->debug("[Network] Received {} bytes from {}", bytes, host);
    }
}

void NetworkLogger::log_error(const std::string& error_msg) {
    if (logger_) {
        logger_->error("[Network] Error: {}", error_msg);
    }
}

void NetworkLogger::set_level(spdlog::level::level_enum level) {
    if (logger_) {
        logger_->set_level(level);
    }
}

spdlog::level::level_enum NetworkLogger::get_level() const {
    if (logger_) {
        return logger_->level();
    }
    return spdlog::level::off;
}

void NetworkLogger::flush() {
    if (logger_) {
        logger_->flush();
    }
}

NetworkSimulator::NetworkSimulator() = default;

void NetworkSimulator::connect(const std::string& host, int port) {
    auto& logger = NetworkLogger::instance();
    logger.log_connection(host, port);
    connections_.emplace_back(host, port);
}

void NetworkSimulator::disconnect(const std::string& host, int port) {
    auto& logger = NetworkLogger::instance();
    logger.log_disconnection(host, port);
    
    auto it = std::remove_if(connections_.begin(), connections_.end(),
        [&](const std::pair<std::string, int>& conn) {
            return conn.first == host && conn.second == port;
        });
    connections_.erase(it, connections_.end());
}

void NetworkSimulator::send_data(const std::string& host, const std::string& data) {
    auto& logger = NetworkLogger::instance();
    logger.log_data_sent(host, data.size());
}

std::string NetworkSimulator::receive_data(const std::string& host, size_t max_bytes) {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<size_t> dist(1, max_bytes);
    
    size_t bytes_received = dist(rng);
    auto& logger = NetworkLogger::instance();
    logger.log_data_received(host, bytes_received);
    
    return std::string(bytes_received, 'X');
}

size_t NetworkSimulator::get_connections_count() const {
    return connections_.size();
}

void NetworkSimulator::simulate_error(const std::string& error_msg) {
    auto& logger = NetworkLogger::instance();
    logger.log_error(error_msg);
}

void test_network_operations() {
    NetworkSimulator sim;
    sim.connect("192.168.1.1", 8080);
    sim.send_data("192.168.1.1", "Hello, World!");
    sim.receive_data("192.168.1.1", 1024);
    sim.disconnect("192.168.1.1", 8080);
}

int get_module_id() {
    return MODULE_ID;
}

}
