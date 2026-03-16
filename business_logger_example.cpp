// 业务日志管理器使用示例
// 演示如何使用 BusinessLoggerManager 记录不同业务的日志

#include <spdlog/spdlog.h>
#include <spdlog/sinks/business_logger_manager.h>

#include <iostream>
#include <thread>
#include <chrono>

using namespace spdlog;

// 示例1：基本使用 - 使用便捷函数
void basic_usage_example() {
    std::cout << "=== 示例1：基本使用 ===" << std::endl;
    
    // 初始化业务日志管理器，设置日志文件保存路径
    business_log::initialize("logs/business");
    
    // 使用便捷函数记录各业务日志
    business_log::screen_record(level::info, "开始录制屏幕");
    business_log::screen_record(level::debug, "屏幕分辨率: 1920x1080");
    
    business_log::desktop_open(level::info, "打开桌面");
    business_log::desktop_open(level::warn, "桌面加载较慢");
    
    business_log::keyboard_record(level::info, "开始录制键盘输入");
    business_log::keyboard_record(level::err, "键盘设备未找到");
    
    business_log::sound_record(level::info, "开始录制声音");
    business_log::sound_record(level::critical, "麦克风权限被拒绝");
    
    std::cout << "基本使用示例完成，请查看 logs/business/ 目录下的日志文件" << std::endl;
}

// 示例2：使用格式化字符串
void formatted_log_example() {
    std::cout << "\n=== 示例2：格式化日志 ===" << std::endl;
    
    // 使用格式化模板函数
    business_log::screen_record_fmt(level::info, "屏幕录制开始，帧率: {} fps", 60);
    business_log::screen_record_fmt(level::debug, "捕获区域: ({}, {}) - ({}, {})", 0, 0, 1920, 1080);
    
    business_log::desktop_open_fmt(level::info, "桌面窗口句柄: {}", 0x123456);
    business_log::desktop_open_fmt(level::warn, "加载超时: {}ms", 5000);
    
    business_log::keyboard_record_fmt(level::info, "捕获按键: {}", "Ctrl+C");
    business_log::keyboard_record_fmt(level::debug, "按键码: {}, 状态: {}", 67, "按下");
    
    business_log::sound_record_fmt(level::info, "音频采样率: {} Hz", 44100);
    business_log::sound_record_fmt(level::debug, "声道数: {}, 位深: {} bit", 2, 16);
    
    std::cout << "格式化日志示例完成" << std::endl;
}

// 示例3：使用 Logger 对象直接操作
void logger_object_example() {
    std::cout << "\n=== 示例3：使用Logger对象 ===" << std::endl;
    
    auto& manager = sinks::BusinessLoggerManager::get_instance();
    
    // 获取各业务的logger
    auto screen_logger = manager.get_logger(sinks::BusinessType::ScreenRecord);
    auto desktop_logger = manager.get_logger(sinks::BusinessType::DesktopOpen);
    auto keyboard_logger = manager.get_logger(sinks::BusinessType::KeyboardRecord);
    auto sound_logger = manager.get_logger(sinks::BusinessType::SoundRecord);
    
    // 使用logger对象记录日志
    screen_logger->info("通过logger对象记录屏幕日志");
    desktop_logger->warn("通过logger对象记录桌面日志");
    keyboard_logger->error("通过logger对象记录键盘日志");
    sound_logger->info("通过logger对象记录声音日志");
    
    std::cout << "Logger对象示例完成" << std::endl;
}

// 示例4：多线程日志记录
void multi_thread_example() {
    std::cout << "\n=== 示例4：多线程日志记录 ===" << std::endl;
    
    auto& manager = sinks::BusinessLoggerManager::get_instance();
    
    // 创建多个线程同时记录不同业务的日志
    std::thread t1([&]() {
        for (int i = 0; i < 10; ++i) {
            business_log::screen_record_fmt(level::info, "线程1 - 屏幕录制帧 {}", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    std::thread t2([&]() {
        for (int i = 0; i < 10; ++i) {
            business_log::keyboard_record_fmt(level::info, "线程2 - 键盘事件 {}", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    std::thread t3([&]() {
        for (int i = 0; i < 10; ++i) {
            business_log::sound_record_fmt(level::info, "线程3 - 音频采样 {}", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    std::cout << "多线程日志记录示例完成" << std::endl;
}

// 示例5：日志级别控制
void log_level_example() {
    std::cout << "\n=== 示例5：日志级别控制 ===" << std::endl;
    
    auto& manager = sinks::BusinessLoggerManager::get_instance();
    
    // 设置所有业务logger的日志级别为warning
    manager.set_level(level::warn);
    
    std::cout << "设置日志级别为warning，以下info和debug日志不应显示" << std::endl;
    
    business_log::screen_record(level::info, "这条info日志不应显示");
    business_log::screen_record(level::debug, "这条debug日志不应显示");
    business_log::screen_record(level::warn, "这条warn日志应该显示");
    business_log::screen_record(level::err, "这条error日志应该显示");
    
    // 恢复为info级别
    manager.set_level(level::info);
    business_log::screen_record(level::info, "恢复info级别，这条日志应该显示");
    
    std::cout << "日志级别控制示例完成" << std::endl;
}

// 示例6：模拟实际业务场景
void real_world_example() {
    std::cout << "\n=== 示例6：实际业务场景模拟 ===" << std::endl;
    
    auto& manager = sinks::BusinessLoggerManager::get_instance();
    
    // 模拟屏幕录制业务
    auto screen_logger = manager.get_logger(sinks::BusinessType::ScreenRecord);
    screen_logger->info("========== 屏幕录制会话开始 ==========");
    screen_logger->info("初始化录制参数: 分辨率=1920x1080, 帧率=30fps, 编码器=H.264");
    screen_logger->debug("创建录制缓冲区: 大小=10MB");
    screen_logger->info("开始捕获屏幕");
    
    for (int i = 0; i < 5; ++i) {
        screen_logger->debug("捕获帧 #{}，大小={}KB", i, 512);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    screen_logger->warn("检测到帧率下降，当前帧率=25fps");
    screen_logger->info("停止捕获屏幕");
    screen_logger->info("录制文件保存至: /recordings/screen_20240316.mp4");
    screen_logger->info("========== 屏幕录制会话结束 ==========");
    
    // 模拟键盘录制业务
    auto keyboard_logger = manager.get_logger(sinks::BusinessType::KeyboardRecord);
    keyboard_logger->info("========== 键盘录制会话开始 ==========");
    keyboard_logger->info("监听键盘设备: /dev/input/event0");
    keyboard_logger->debug("设置键盘事件过滤: 仅记录可打印字符");
    
    const char* keys[] = {"Ctrl", "Alt", "T", "H", "E", "Enter"};
    for (const auto& key : keys) {
        keyboard_logger->debug("按键事件: key={}, action=按下", key);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        keyboard_logger->debug("按键事件: key={}, action=释放", key);
    }
    
    keyboard_logger->info("停止键盘监听");
    keyboard_logger->info("========== 键盘录制会话结束 ==========");
    
    std::cout << "实际业务场景模拟完成" << std::endl;
}

// 示例7：日志轮转测试
void rotation_test_example() {
    std::cout << "\n=== 示例7：日志轮转测试 ===" << std::endl;
    std::cout << "注意：此示例会生成大量日志数据用于测试轮转功能" << std::endl;
    
    auto& manager = sinks::BusinessLoggerManager::get_instance();
    auto screen_logger = manager.get_logger(sinks::BusinessType::ScreenRecord);
    
    // 生成大量日志以触发轮转（30MB * 3 = 最多90MB数据）
    // 为了测试，我们生成一些较大的日志消息
    std::string large_msg(1000, 'X');  // 1KB的消息
    
    screen_logger->info("开始日志轮转测试...");
    
    // 写入足够多的日志以触发轮转
    for (int i = 0; i < 1000; ++i) {
        screen_logger->info("轮转测试消息 #{}: {}", i, large_msg);
    }
    
    screen_logger->info("日志轮转测试完成");
    
    std::cout << "日志轮转测试完成，请检查 logs/business/screen_record.log* 文件" << std::endl;
}

int main() {
    try {
        std::cout << "========================================" << std::endl;
        std::cout << "   业务日志管理器使用示例" << std::endl;
        std::cout << "========================================" << std::endl;
        
        // 运行所有示例
        basic_usage_example();
        formatted_log_example();
        logger_object_example();
        multi_thread_example();
        log_level_example();
        real_world_example();
        // rotation_test_example();  // 取消注释以测试日志轮转
        
        // 刷新所有日志
        auto& manager = sinks::BusinessLoggerManager::get_instance();
        manager.flush_all();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "   所有示例执行完成！" << std::endl;
        std::cout << "   请查看 logs/business/ 目录下的日志文件" << std::endl;
        std::cout << "========================================" << std::endl;
        
        // 关闭日志管理器
        manager.shutdown();
        
        return 0;
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "日志系统错误: " << ex.what() << std::endl;
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "错误: " << ex.what() << std::endl;
        return 1;
    }
}
