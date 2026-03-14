#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>
#include <mutex>

namespace spdlog {

enum class ModuleType {
    kNetwork,
    kStorage,
    kUI
};

class ModuleLogger {
public:
    static ModuleLogger& Instance();

    void Init(const std::string& log_dir = "logs");
    void Shutdown();

    void Log(ModuleType module, level::level_enum lvl, const std::string& msg);

    template <typename... Args>
    void Log(ModuleType module, level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args);

    std::shared_ptr<logger> GetLogger(ModuleType module);

private:
    ModuleLogger() = default;
    ~ModuleLogger() = default;
    ModuleLogger(const ModuleLogger&) = delete;
    ModuleLogger& operator=(const ModuleLogger&) = delete;

    std::shared_ptr<sinks::rotating_file_sink_mt> CreateRotatingSink(const std::string& filename);
    std::string GetModuleName(ModuleType module);
    std::string GetModuleLogFile(ModuleType module);

    bool initialized_ = false;
    std::string log_dir_;
    std::shared_ptr<logger> network_logger_;
    std::shared_ptr<logger> storage_logger_;
    std::shared_ptr<logger> ui_logger_;
    std::shared_ptr<sinks::rotating_file_sink_mt> global_sink_;
    std::mutex init_mutex_;
};

template <typename... Args>
void ModuleLogger::Log(ModuleType module, level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
    auto logger = GetLogger(module);
    if (logger) {
        logger->log(lvl, fmt, std::forward<Args>(args)...);
    }
}

#define NETWORK_LOG_TRACE(...)   spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kNetwork, spdlog::level::trace, __VA_ARGS__)
#define NETWORK_LOG_DEBUG(...)   spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kNetwork, spdlog::level::debug, __VA_ARGS__)
#define NETWORK_LOG_INFO(...)    spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kNetwork, spdlog::level::info, __VA_ARGS__)
#define NETWORK_LOG_WARN(...)    spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kNetwork, spdlog::level::warn, __VA_ARGS__)
#define NETWORK_LOG_ERROR(...)   spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kNetwork, spdlog::level::err, __VA_ARGS__)
#define NETWORK_LOG_CRITICAL(...) spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kNetwork, spdlog::level::critical, __VA_ARGS__)

#define STORAGE_LOG_TRACE(...)   spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kStorage, spdlog::level::trace, __VA_ARGS__)
#define STORAGE_LOG_DEBUG(...)   spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kStorage, spdlog::level::debug, __VA_ARGS__)
#define STORAGE_LOG_INFO(...)    spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kStorage, spdlog::level::info, __VA_ARGS__)
#define STORAGE_LOG_WARN(...)    spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kStorage, spdlog::level::warn, __VA_ARGS__)
#define STORAGE_LOG_ERROR(...)   spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kStorage, spdlog::level::err, __VA_ARGS__)
#define STORAGE_LOG_CRITICAL(...) spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kStorage, spdlog::level::critical, __VA_ARGS__)

#define UI_LOG_TRACE(...)   spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kUI, spdlog::level::trace, __VA_ARGS__)
#define UI_LOG_DEBUG(...)   spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kUI, spdlog::level::debug, __VA_ARGS__)
#define UI_LOG_INFO(...)    spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kUI, spdlog::level::info, __VA_ARGS__)
#define UI_LOG_WARN(...)    spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kUI, spdlog::level::warn, __VA_ARGS__)
#define UI_LOG_ERROR(...)   spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kUI, spdlog::level::err, __VA_ARGS__)
#define UI_LOG_CRITICAL(...) spdlog::ModuleLogger::Instance().Log(spdlog::ModuleType::kUI, spdlog::level::critical, __VA_ARGS__)

}

#include "module_logger-inl.h"
