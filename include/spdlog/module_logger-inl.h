#pragma once

#include "module_logger.h"
#include <spdlog/details/os.h>
#include <vector>

namespace spdlog {

ModuleLogger& ModuleLogger::Instance() {
    static ModuleLogger instance;
    return instance;
}

void ModuleLogger::Init(const std::string& log_dir) {
    std::lock_guard<std::mutex> lock(init_mutex_);
    if (initialized_) {
        return;
    }

    log_dir_ = log_dir;

    details::os::create_dir(log_dir_);

    global_sink_ = CreateRotatingSink(log_dir_ + "/global.log");
    global_sink_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%L%$] [%n] [thread %t] %v");

    auto network_sink = CreateRotatingSink(GetModuleLogFile(ModuleType::kNetwork));
    network_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%L%$] [network] [thread %t] %v");
    network_logger_ = std::make_shared<logger>("network");
    network_logger_->sinks().push_back(network_sink);
    network_logger_->sinks().push_back(global_sink_);
    network_logger_->set_level(level::trace);
    network_logger_->flush_on(level::err);
    register_logger(network_logger_);

    auto storage_sink = CreateRotatingSink(GetModuleLogFile(ModuleType::kStorage));
    storage_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%L%$] [storage] [thread %t] %v");
    storage_logger_ = std::make_shared<logger>("storage");
    storage_logger_->sinks().push_back(storage_sink);
    storage_logger_->sinks().push_back(global_sink_);
    storage_logger_->set_level(level::trace);
    storage_logger_->flush_on(level::err);
    register_logger(storage_logger_);

    auto ui_sink = CreateRotatingSink(GetModuleLogFile(ModuleType::kUI));
    ui_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%L%$] [ui] [thread %t] %v");
    ui_logger_ = std::make_shared<logger>("ui");
    ui_logger_->sinks().push_back(ui_sink);
    ui_logger_->sinks().push_back(global_sink_);
    ui_logger_->set_level(level::trace);
    ui_logger_->flush_on(level::err);
    register_logger(ui_logger_);

    initialized_ = true;
}

void ModuleLogger::Shutdown() {
    std::lock_guard<std::mutex> lock(init_mutex_);
    if (!initialized_) {
        return;
    }

    apply_all([](std::shared_ptr<logger> l) {
        l->flush();
    });

    shutdown();

    network_logger_.reset();
    storage_logger_.reset();
    ui_logger_.reset();
    global_sink_.reset();

    initialized_ = false;
}

void ModuleLogger::Log(ModuleType module, level::level_enum lvl, const std::string& msg) {
    auto logger = GetLogger(module);
    if (logger) {
        logger->log(lvl, msg);
    }
}

std::shared_ptr<logger> ModuleLogger::GetLogger(ModuleType module) {
    if (!initialized_) {
        Init();
    }

    switch (module) {
        case ModuleType::kNetwork:
            return network_logger_;
        case ModuleType::kStorage:
            return storage_logger_;
        case ModuleType::kUI:
            return ui_logger_;
        default:
            return nullptr;
    }
}

std::shared_ptr<sinks::rotating_file_sink_mt> ModuleLogger::CreateRotatingSink(const std::string& filename) {
    const size_t max_file_size = 5 * 1024 * 1024;
    const size_t max_files = 3;
    return std::make_shared<sinks::rotating_file_sink_mt>(filename, max_file_size, max_files);
}

std::string ModuleLogger::GetModuleName(ModuleType module) {
    switch (module) {
        case ModuleType::kNetwork:
            return "network";
        case ModuleType::kStorage:
            return "storage";
        case ModuleType::kUI:
            return "ui";
        default:
            return "unknown";
    }
}

std::string ModuleLogger::GetModuleLogFile(ModuleType module) {
    return log_dir_ + "/" + GetModuleName(module) + ".log";
}

class AutoInitShutdown {
public:
    AutoInitShutdown() {
        ModuleLogger::Instance().Init();
    }

    ~AutoInitShutdown() {
        ModuleLogger::Instance().Shutdown();
    }
};

static AutoInitShutdown auto_init_shutdown;

}
