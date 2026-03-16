// 简单的业务日志管理器测试程序
// 直接编译运行，不依赖 CMake

#define SPDLOG_HEADER_ONLY
#include "include/spdlog/spdlog.h"
#include "include/spdlog/sinks/business_logger_manager-inl.h"

#include <iostream>

int main() {
    try {
        std::cout << "========================================" << std::endl;
        std::cout << "   业务日志管理器测试" << std::endl;
        std::cout << "========================================" << std::endl;
        
        // 初始化业务日志管理器
        std::cout << "初始化日志管理器..." << std::endl;
        spdlog::business_log::initialize("logs/business");
        
        // 测试1: 基本日志记录
        std::cout << "\n测试1: 基本日志记录" << std::endl;
        spdlog::business_log::screen_record(spdlog::level::info, "屏幕录制开始");
        spdlog::business_log::desktop_open(spdlog::level::info, "桌面已打开");
        spdlog::business_log::keyboard_record(spdlog::level::info, "键盘录制开始");
        spdlog::business_log::sound_record(spdlog::level::info, "声音录制开始");
        
        // 测试2: 格式化日志
        std::cout << "测试2: 格式化日志" << std::endl;
        spdlog::business_log::screen_record_fmt(spdlog::level::info, "分辨率: {}x{}", 1920, 1080);
        spdlog::business_log::desktop_open_fmt(spdlog::level::warn, "窗口句柄: 0x{:X}", 0x123456);
        spdlog::business_log::keyboard_record_fmt(spdlog::level::debug, "按键: {}, 状态: {}", "Ctrl+C", "按下");
        spdlog::business_log::sound_record_fmt(spdlog::level::info, "采样率: {} Hz, 声道: {}", 44100, 2);
        
        // 测试3: 不同日志级别
        std::cout << "测试3: 不同日志级别" << std::endl;
        auto& manager = spdlog::sinks::BusinessLoggerManager::get_instance();
        auto screen_logger = manager.get_logger(spdlog::sinks::BusinessType::ScreenRecord);
        screen_logger->trace("Trace 消息");
        screen_logger->debug("Debug 消息");
        screen_logger->info("Info 消息");
        screen_logger->warn("Warn 消息");
        screen_logger->error("Error 消息");
        screen_logger->critical("Critical 消息");
        
        // 刷新所有日志
        std::cout << "\n刷新所有日志..." << std::endl;
        manager.flush_all();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "   测试完成！" << std::endl;
        std::cout << "   请查看 logs/business/ 目录下的日志文件:" << std::endl;
        std::cout << "   - screen_record.log" << std::endl;
        std::cout << "   - desktop_open.log" << std::endl;
        std::cout << "   - keyboard_record.log" << std::endl;
        std::cout << "   - sound_record.log" << std::endl;
        std::cout << "========================================" << std::endl;
        
        // 关闭日志管理器
        manager.shutdown();
        
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "错误: " << ex.what() << std::endl;
        return 1;
    }
}
