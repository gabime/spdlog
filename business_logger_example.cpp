// business_logger_example.cpp
// 多业务日志管理器使用示例

#include "business_logger.h"
#include <iostream>
#include <thread>
#include <chrono>

// 示例1: 基本使用
void basic_usage_example() {
    std::cout << "=== 基本使用示例 ===" << std::endl;

    // 初始化日志系统（使用默认配置）
    business::init_logger();

    // 使用便捷宏记录不同业务的日志
    LOG_SCREEN_RECORD(spdlog::level::info, "开始录制屏幕，分辨率: {}x{}", 1920, 1080);
    LOG_SCREEN_RECORD(spdlog::level::debug, "屏幕录制帧率: {} fps", 60);

    LOG_DESKTOP_OPEN(spdlog::level::info, "打开桌面窗口: {}", "主桌面");
    LOG_DESKTOP_OPEN(spdlog::level::warn, "桌面窗口已最小化");

    LOG_KEYBOARD_RECORD(spdlog::level::info, "开始录制键盘输入");
    LOG_KEYBOARD_RECORD(spdlog::level::debug, "捕获按键: {}", "Ctrl+C");

    LOG_SOUND_RECORD(spdlog::level::info, "开始录制音频，采样率: {} Hz", 44100);
    LOG_SOUND_RECORD(spdlog::level::err, "音频设备初始化失败: {}", "设备未找到");

    // 刷新所有日志
    business::BusinessLogger::instance().flush_all();
}

// 示例2: 自定义配置
void custom_config_example() {
    std::cout << "\n=== 自定义配置示例 ===" << std::endl;

    // 创建自定义配置
    business::LogConfig config;
    config.log_dir = "custom_logs";                    // 自定义日志目录
    config.max_file_size = 30 * 1024 * 1024;          // 30M
    config.max_files = 3;                              // 最多3个文件
    config.enable_console = true;                      // 同时输出到控制台
    config.level = spdlog::level::debug;               // 调试级别

    // 初始化
    business::init_logger(config);

    // 使用 API 方式记录日志
    auto screen_logger = business::get_business_logger(business::BusinessType::ScreenRecord);
    screen_logger->info("使用 logger 对象直接记录日志");
    screen_logger->debug("调试信息: 当前录制时间: {} 秒", 120);

    // 使用业务类型直接记录
    business::log_business(business::BusinessType::SoundRecord, spdlog::level::info, "直接记录音频日志");
}

// 示例3: 多线程使用
void multi_thread_example() {
    std::cout << "\n=== 多线程使用示例 ===" << std::endl;

    business::LogConfig config;
    config.log_dir = "logs";
    config.max_file_size = 30 * 1024 * 1024;
    config.max_files = 3;
    config.enable_console = false;  // 多线程测试时关闭控制台输出
    business::init_logger(config);

    // 创建多个线程同时记录不同业务的日志
    std::thread t1([]() {
        for (int i = 0; i < 10; ++i) {
            LOG_SCREEN_RECORD(spdlog::level::info, "屏幕录制线程 - 第 {} 帧", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    std::thread t2([]() {
        for (int i = 0; i < 10; ++i) {
            LOG_KEYBOARD_RECORD(spdlog::level::info, "键盘录制线程 - 按键事件 {}", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    std::thread t3([]() {
        for (int i = 0; i < 10; ++i) {
            LOG_SOUND_RECORD(spdlog::level::info, "音频录制线程 - 采样 {}", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    t1.join();
    t2.join();
    t3.join();

    std::cout << "多线程日志记录完成，请查看 logs 目录下的日志文件" << std::endl;
}

// 示例4: 日志级别控制
void log_level_example() {
    std::cout << "\n=== 日志级别控制示例 ===" << std::endl;

    business::LogConfig config;
    config.log_dir = "logs";
    config.level = spdlog::level::info;  // 设置默认级别为 info
    business::init_logger(config);

    auto logger = business::get_business_logger(business::BusinessType::DesktopOpen);

    logger->trace("这条 trace 消息不会显示（级别不够）");
    logger->debug("这条 debug 消息不会显示（级别不够）");
    logger->info("这条 info 消息会显示");
    logger->warn("这条 warn 消息会显示");
    logger->error("这条 error 消息会显示");

    // 动态修改日志级别
    std::cout << "修改日志级别为 debug..." << std::endl;
    business::BusinessLogger::instance().set_level(spdlog::level::debug);

    logger->debug("现在这条 debug 消息会显示了");
    logger->trace("但 trace 消息仍然不会显示");
}

// 示例5: 大文件轮转测试
void rotation_test_example() {
    std::cout << "\n=== 日志轮转测试示例 ===" << std::endl;

    // 配置小文件大小以便快速测试轮转
    business::LogConfig config;
    config.log_dir = "rotation_test_logs";
    config.max_file_size = 1024;  // 1KB 方便测试
    config.max_files = 3;
    config.enable_console = false;
    business::init_logger(config);

    auto logger = business::get_business_logger(business::BusinessType::ScreenRecord);

    // 写入足够多的日志以触发轮转
    std::cout << "正在写入日志以触发轮转（约 5KB 数据）..." << std::endl;
    for (int i = 0; i < 100; ++i) {
        logger->info("这是一条测试日志消息，用于测试日志轮转功能。序号: {:04d}", i);
    }

    logger->flush();
    std::cout << "日志写入完成，请查看 rotation_test_logs 目录下的文件" << std::endl;
    std::cout << "预期文件: screen_record.log, screen_record.1.log, screen_record.2.log, screen_record.3.log" << std::endl;
}

// 示例6: 实际业务场景模拟
void real_world_example() {
    std::cout << "\n=== 实际业务场景模拟 ===" << std::endl;

    business::LogConfig config;
    config.log_dir = "business_logs";
    config.max_file_size = 30 * 1024 * 1024;  // 30M
    config.max_files = 3;
    config.enable_console = true;
    config.level = spdlog::level::info;
    business::init_logger(config);

    // 模拟屏幕录制业务
    LOG_SCREEN_RECORD(spdlog::level::info, "[屏幕录制] 开始录制 - 任务ID: {}", "SR001");
    LOG_SCREEN_RECORD(spdlog::level::info, "[屏幕录制] 设置录制参数 - 分辨率: {}x{}, 帧率: {}, 编码: {}", 
                      1920, 1080, 30, "H.264");
    LOG_SCREEN_RECORD(spdlog::level::debug, "[屏幕录制] 初始化编码器...");
    LOG_SCREEN_RECORD(spdlog::level::info, "[屏幕录制] 录制中...");
    LOG_SCREEN_RECORD(spdlog::level::warn, "[屏幕录制] 检测到帧率下降，当前帧率: {}", 28);
    LOG_SCREEN_RECORD(spdlog::level::info, "[屏幕录制] 录制完成 - 总时长: {} 秒, 文件大小: {} MB", 300, 150);

    // 模拟打开桌面业务
    LOG_DESKTOP_OPEN(spdlog::level::info, "[桌面管理] 打开桌面 - 桌面ID: {}", "DESK001");
    LOG_DESKTOP_OPEN(spdlog::level::info, "[桌面管理] 加载桌面配置...");
    LOG_DESKTOP_OPEN(spdlog::level::debug, "[桌面管理] 桌面分辨率: {}x{}", 1920, 1080);
    LOG_DESKTOP_OPEN(spdlog::level::info, "[桌面管理] 桌面已就绪");

    // 模拟键盘录制业务
    LOG_KEYBOARD_RECORD(spdlog::level::info, "[键盘录制] 开始监听键盘输入 - 会话ID: {}", "KB001");
    LOG_KEYBOARD_RECORD(spdlog::level::debug, "[键盘录制] 设置钩子...");
    LOG_KEYBOARD_RECORD(spdlog::level::info, "[键盘录制] 捕获按键 - KeyCode: {}, KeyName: {}", 13, "Enter");
    LOG_KEYBOARD_RECORD(spdlog::level::info, "[键盘录制] 捕获按键 - KeyCode: {}, KeyName: {}", 65, "A");
    LOG_KEYBOARD_RECORD(spdlog::level::warn, "[键盘录制] 检测到组合键: {}", "Ctrl+Alt+Del");
    LOG_KEYBOARD_RECORD(spdlog::level::info, "[键盘录制] 停止监听");

    // 模拟声音录制业务
    LOG_SOUND_RECORD(spdlog::level::info, "[音频录制] 开始录制 - 任务ID: {}", "AUD001");
    LOG_SOUND_RECORD(spdlog::level::info, "[音频录制] 音频参数 - 采样率: {} Hz, 位深: {} bit, 声道: {}", 44100, 16, 2);
    LOG_SOUND_RECORD(spdlog::level::debug, "[音频录制] 初始化音频设备...");
    LOG_SOUND_RECORD(spdlog::level::info, "[音频录制] 音频设备就绪: {}", "麦克风 (Realtek)");
    LOG_SOUND_RECORD(spdlog::level::err, "[音频录制] 音频缓冲区溢出，丢失 {} 个采样", 512);
    LOG_SOUND_RECORD(spdlog::level::info, "[音频录制] 录制完成 - 总时长: {} 秒", 180);

    business::BusinessLogger::instance().flush_all();
    std::cout << "业务场景模拟完成，请查看 business_logs 目录下的日志文件" << std::endl;
}

int main() {
    try {
        std::cout << "========================================" << std::endl;
        std::cout << "   多业务日志管理器使用示例" << std::endl;
        std::cout << "========================================" << std::endl;

        // 运行各个示例
        basic_usage_example();
        custom_config_example();
        multi_thread_example();
        log_level_example();
        rotation_test_example();
        real_world_example();

        std::cout << "\n========================================" << std::endl;
        std::cout << "   所有示例运行完成" << std::endl;
        std::cout << "========================================" << std::endl;

        // 关闭日志系统
        business::BusinessLogger::instance().shutdown();

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "错误: " << ex.what() << std::endl;
        return 1;
    }
}
