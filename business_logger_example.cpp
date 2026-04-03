// 多业务日志系统使用示例
// 演示如何使用business_logger.h实现多业务日志记录

#include "business_logger.h"
#include <iostream>
#include <thread>
#include <chrono>

// 模拟录制屏幕业务
void simulate_screen_recording() {
    business_log::info(business_log::BusinessType::Screen, "开始录制屏幕");
    
    for (int i = 0; i < 5; ++i) {
        business_log::debug(business_log::BusinessType::Screen, 
            "屏幕录制帧 #{} - 分辨率: 1920x1080, 编码: H.264", i + 1);
        
        if (i % 2 == 0) {
            business_log::warn(business_log::BusinessType::Screen, 
                "屏幕录制帧 #{} 处理延迟 {}ms", i + 1, 16);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    business_log::info(business_log::BusinessType::Screen, "屏幕录制完成，共录制 {} 帧", 5);
}

// 模拟打开桌面业务
void simulate_desktop_access() {
    business_log::info(business_log::BusinessType::Desktop, "开始访问桌面");
    
    business_log::debug(business_log::BusinessType::Desktop, "获取桌面窗口句柄成功");
    business_log::debug(business_log::BusinessType::Desktop, "桌面分辨率: 1920x1080");
    business_log::debug(business_log::BusinessType::Desktop, "桌面颜色深度: 32位");
    
    // 模拟错误情况
    business_log::error(business_log::BusinessType::Desktop, 
        "无法获取桌面壁纸路径: 系统错误码 {}", 2);
    
    business_log::info(business_log::BusinessType::Desktop, "桌面访问结束");
}

// 模拟录制键盘业务
void simulate_keyboard_recording() {
    business_log::info(business_log::BusinessType::Keyboard, "开始录制键盘输入");
    
    const char* keys[] = {"A", "B", "Enter", "Ctrl+C", "Esc"};
    for (int i = 0; i < 5; ++i) {
        business_log::debug(business_log::BusinessType::Keyboard, 
            "捕获按键: {}, 虚拟键码: {}", keys[i], 65 + i);
    }
    
    business_log::warn(business_log::BusinessType::Keyboard, 
        "检测到连续快速按键，可能存在键盘记录器冲突");
    
    business_log::info(business_log::BusinessType::Keyboard, 
        "键盘录制完成，共捕获 {} 个按键事件", 5);
}

// 模拟录制声音业务
void simulate_audio_recording() {
    business_log::info(business_log::BusinessType::Audio, "开始录制声音");
    
    business_log::debug(business_log::BusinessType::Audio, 
        "音频设备: 默认麦克风 (Realtek Audio)");
    business_log::debug(business_log::BusinessType::Audio, 
        "采样率: 44100 Hz, 位深度: 16位, 声道: 立体声");
    
    for (int i = 0; i < 3; ++i) {
        business_log::debug(business_log::BusinessType::Audio, 
            "录制音频块 #{} - 大小: {} bytes", i + 1, 4096);
    }
    
    business_log::info(business_log::BusinessType::Audio, 
        "声音录制完成，总时长: {} 秒", 3);
}

// 演示日志轮转功能
void demonstrate_rotation() {
    std::cout << "\n=== 演示日志轮转功能 ===" << std::endl;
    
    business_log::info(business_log::BusinessType::Screen, 
        "这是一条普通日志，用于测试轮转前的写入");
    
    // 手动触发轮转
    business_log::rotate_now(business_log::BusinessType::Screen);
    
    business_log::info(business_log::BusinessType::Screen, 
        "轮转后的第一条日志");
    
    std::cout << "日志轮转完成，请查看 logs/screen.log 和 logs/screen.1.log" << std::endl;
}

// 演示多线程写入
void demonstrate_multithread() {
    std::cout << "\n=== 演示多线程写入 ===" << std::endl;
    
    std::thread t1([]() {
        for (int i = 0; i < 10; ++i) {
            business_log::info(business_log::BusinessType::Screen, 
                "线程1 - 屏幕日志 #{}", i);
        }
    });
    
    std::thread t2([]() {
        for (int i = 0; i < 10; ++i) {
            business_log::info(business_log::BusinessType::Keyboard, 
                "线程2 - 键盘日志 #{}", i);
        }
    });
    
    std::thread t3([]() {
        for (int i = 0; i < 10; ++i) {
            business_log::info(business_log::BusinessType::Audio, 
                "线程3 - 音频日志 #{}", i);
        }
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    std::cout << "多线程写入完成" << std::endl;
}

// 演示大量日志写入（测试文件轮转）
void demonstrate_large_logs() {
    std::cout << "\n=== 演示大量日志写入（测试30MB限制） ===" << std::endl;
    std::cout << "正在写入大量日志，请稍候..." << std::endl;
    
    // 写入足够多的日志以触发轮转（每条约100字节，需要约30万条达到30MB）
    for (int i = 0; i < 10000; ++i) {
        business_log::info(business_log::BusinessType::Desktop, 
            "这是一条用于测试日志轮转的长日志消息 - 序号: {}, 时间戳: {}, 附加信息: {}",
            i, 
            "2024-01-01 12:00:00",
            "abcdefghijklmnopqrstuvwxyz");
    }
    
    std::cout << "大量日志写入完成" << std::endl;
}

int main() {
    try {
        std::cout << "=== 多业务日志系统示例 ===" << std::endl;
        
        // 1. 初始化日志系统
        // 配置：日志目录为logs，单个文件最大30MB，最多保留3个文件
        business_log::LogConfig config;
        config.log_dir = "logs";
        config.max_file_size = 30 * 1024 * 1024;  // 30MB
        config.max_files = 3;
        config.rotate_on_open = false;
        
        business_log::init(config);
        
        std::cout << "日志系统初始化完成" << std::endl;
        std::cout << "日志目录: " << config.log_dir << std::endl;
        std::cout << "最大文件大小: " << config.max_file_size / 1024 / 1024 << " MB" << std::endl;
        std::cout << "最大文件数: " << config.max_files << std::endl;
        
        // 2. 模拟各业务日志
        std::cout << "\n=== 模拟各业务日志 ===" << std::endl;
        
        simulate_screen_recording();
        simulate_desktop_access();
        simulate_keyboard_recording();
        simulate_audio_recording();
        
        // 3. 演示日志级别设置
        std::cout << "\n=== 设置日志级别 ===" << std::endl;
        business_log::set_level(business_log::BusinessType::Screen, spdlog::level::debug);
        std::cout << "屏幕录制日志级别设置为 DEBUG" << std::endl;
        
        // 4. 演示手动轮转
        demonstrate_rotation();
        
        // 5. 演示多线程写入
        demonstrate_multithread();
        
        // 6. 刷新所有日志
        business_log::flush_all();
        std::cout << "\n所有日志已刷新到磁盘" << std::endl;
        
        // 7. 显示日志文件位置
        std::cout << "\n=== 生成的日志文件 ===" << std::endl;
        std::cout << "屏幕录制日志: logs/screen.log" << std::endl;
        std::cout << "桌面访问日志: logs/desktop.log" << std::endl;
        std::cout << "键盘录制日志: logs/keyboard.log" << std::endl;
        std::cout << "声音录制日志: logs/audio.log" << std::endl;
        
        // 8. 关闭日志系统
        business_log::shutdown();
        std::cout << "\n日志系统已关闭" << std::endl;
        
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "错误: " << ex.what() << std::endl;
        return 1;
    }
}
