//
// 多业务日志示例
// 演示如何使用 business_sink 实现按业务分类的日志记录
//

#include <spdlog/spdlog.h>
#include <spdlog/sinks/business_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <iostream>
#include <thread>
#include <chrono>

// 使用 spdlog 命名空间，但避免 logger 名称冲突
using spdlog::logger;
using spdlog::sink_ptr;
using spdlog::level::level_enum;
using spdlog::sinks::business_sink_mt;
using spdlog::sinks::stdout_color_sink_mt;
using spdlog::sinks::BusinessType;

// 方式1: 使用独立的 logger 名称来区分业务
void example1_separate_loggers() {
    std::cout << "\n=== 示例1: 使用独立的 logger 名称 ===" << std::endl;
    
    // 创建业务分类 sink
    auto biz_sink = std::make_shared<business_sink_mt>("logs/business", 30 * 1024 * 1024, 3);
    
    // 为每个业务创建独立的 logger
    auto screen_logger = std::make_shared<logger>("screen_logger", biz_sink);
    auto desktop_logger = std::make_shared<logger>("desktop_logger", biz_sink);
    auto keyboard_logger = std::make_shared<logger>("keyboard_logger", biz_sink);
    auto audio_logger = std::make_shared<logger>("audio_logger", biz_sink);
    
    // 写入不同业务的日志
    screen_logger->info("开始录制屏幕");
    screen_logger->info("屏幕分辨率: 1920x1080");
    screen_logger->info("帧率: 30fps");
    
    desktop_logger->info("打开桌面");
    desktop_logger->info("桌面路径: C:\\Users\\Admin\\Desktop");
    
    keyboard_logger->info("开始录制键盘");
    keyboard_logger->debug("捕获按键: Ctrl+C");
    
    audio_logger->info("开始录制声音");
    audio_logger->info("采样率: 44100Hz");
    audio_logger->info("声道: 立体声");
    
    // 刷新所有日志
    biz_sink->flush();
    
    std::cout << "日志已写入 logs/business/ 目录下的各个业务日志文件" << std::endl;
}

// 方式2: 使用业务类型标记写入
void example2_business_type_markers() {
    std::cout << "\n=== 示例2: 使用业务类型标记 ===" << std::endl;
    
    // 创建业务分类 sink
    auto biz_sink = std::make_shared<business_sink_mt>("logs/business2", 30 * 1024 * 1024, 3);
    
    // 创建一个通用的 logger
    auto app_logger = std::make_shared<logger>("app_logger", biz_sink);
    
    // 通过修改 logger 名称来切换业务类型
    // 注意：实际使用时可以通过宏或包装函数简化
    
    // 屏幕录制业务
    app_logger->info("[SCREEN] 初始化屏幕录制模块");
    app_logger->info("[SCREEN] 设置录制区域: (0,0) - (1920,1080)");
    
    // 桌面操作业务
    app_logger->info("[DESKTOP] 扫描桌面文件");
    app_logger->info("[DESKTOP] 发现 15 个文件");
    
    // 键盘记录业务
    app_logger->info("[KEYBOARD] 安装键盘钩子");
    app_logger->info("[KEYBOARD] 开始监听键盘事件");
    
    // 音频录制业务
    app_logger->info("[AUDIO] 初始化音频设备");
    app_logger->info("[AUDIO] 选择输入设备: 麦克风");
    
    biz_sink->flush();
    
    std::cout << "日志已写入 logs/business2/ 目录" << std::endl;
}

// 方式3: 组合使用 - 控制台 + 业务分类文件
void example3_combined_sinks() {
    std::cout << "\n=== 示例3: 控制台 + 业务分类文件 ===" << std::endl;
    
    // 创建多个 sink
    auto console_sink = std::make_shared<stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);
    console_sink->set_pattern("[%H:%M:%S] [%^%l%$] %v");
    
    auto biz_sink = std::make_shared<business_sink_mt>("logs/combined", 30 * 1024 * 1024, 3);
    
    // 创建同时输出到控制台和业务文件的 logger
    std::vector<sink_ptr> sinks = {console_sink, biz_sink};
    auto multi_logger = std::make_shared<logger>("multi_sink_logger", sinks.begin(), sinks.end());
    multi_logger->set_level(spdlog::level::debug);
    
    // 同时输出到控制台和对应的业务日志文件
    multi_logger->info("应用启动");
    
    // 创建业务特定的 logger
    auto screen_logger = std::make_shared<logger>("screen_recorder", sinks.begin(), sinks.end());
    screen_logger->info("开始屏幕录制");
    screen_logger->info("录制时长: 00:05:32");
    
    auto keyboard_logger = std::make_shared<logger>("keyboard_recorder", sinks.begin(), sinks.end());
    keyboard_logger->info("键盘记录器就绪");
    keyboard_logger->debug("调试模式已启用");
    
    multi_logger->flush();
    
    std::cout << "日志同时输出到控制台和 logs/combined/ 目录" << std::endl;
}

// 方式4: 日志轮转测试
void example4_rotation_test() {
    std::cout << "\n=== 示例4: 日志轮转测试 ===" << std::endl;
    
    // 创建业务分类 sink，设置较小的文件大小以便测试轮转
    // 1MB 大小限制，最多保留 3 个文件
    auto biz_sink = std::make_shared<business_sink_mt>("logs/rotation", 1 * 1024 * 1024, 3);
    
    auto test_logger = std::make_shared<logger>("screen_logger", biz_sink);
    
    std::cout << "正在生成大量日志以测试轮转功能..." << std::endl;
    
    // 生成大量日志，触发轮转
    for (int i = 0; i < 1000; ++i) {
        test_logger->info("这是第 {} 条测试日志消息，用于测试日志轮转功能。", i);
        test_logger->info("附加信息: timestamp={}", 
                     std::chrono::system_clock::now().time_since_epoch().count());
    }
    
    biz_sink->flush();
    
    std::cout << "日志轮转测试完成，检查 logs/rotation/ 目录" << std::endl;
}

// 方式5: 多线程并发写入
void example5_multithreaded() {
    std::cout << "\n=== 示例5: 多线程并发写入 ===" << std::endl;
    
    auto biz_sink = std::make_shared<business_sink_mt>("logs/multithread", 30 * 1024 * 1024, 3);
    
    std::vector<std::thread> threads;
    
    // 创建多个线程，每个线程模拟不同的业务
    auto screen_thread = std::thread([&biz_sink]() {
        auto thread_logger = std::make_shared<logger>("screen_logger", biz_sink);
        for (int i = 0; i < 100; ++i) {
            thread_logger->info("屏幕录制 - 帧 {} 已捕获", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    
    auto keyboard_thread = std::thread([&biz_sink]() {
        auto thread_logger = std::make_shared<logger>("keyboard_logger", biz_sink);
        for (int i = 0; i < 100; ++i) {
            thread_logger->info("键盘记录 - 按键 {} 已记录", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    
    auto audio_thread = std::thread([&biz_sink]() {
        auto thread_logger = std::make_shared<logger>("audio_logger", biz_sink);
        for (int i = 0; i < 100; ++i) {
            thread_logger->info("音频录制 - 缓冲区 {} 已写入", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    
    auto desktop_thread = std::thread([&biz_sink]() {
        auto thread_logger = std::make_shared<logger>("desktop_logger", biz_sink);
        for (int i = 0; i < 100; ++i) {
            thread_logger->info("桌面操作 - 文件 {} 已扫描", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    
    // 等待所有线程完成
    screen_thread.join();
    keyboard_thread.join();
    audio_thread.join();
    desktop_thread.join();
    
    biz_sink->flush();
    
    std::cout << "多线程测试完成，检查 logs/multithread/ 目录" << std::endl;
}

// 方式6: 自定义业务配置
void example6_custom_config() {
    std::cout << "\n=== 示例6: 自定义业务配置 ===" << std::endl;
    
    auto biz_sink = std::make_shared<business_sink_mt>("logs/custom", 30 * 1024 * 1024, 3);
    
    // 自定义业务配置
    biz_sink->set_business_config(
        BusinessType::ScreenRecord, 
        "screen_recorder",
        "logs/custom/screen/screen.log"
    );
    
    biz_sink->set_business_config(
        BusinessType::AudioRecord,
        "audio_recorder", 
        "logs/custom/audio/audio.log"
    );
    
    auto screen_logger = std::make_shared<logger>("screen_logger", biz_sink);
    auto audio_logger = std::make_shared<logger>("audio_logger", biz_sink);
    
    screen_logger->info("使用自定义路径的屏幕日志");
    audio_logger->info("使用自定义路径的音频日志");
    
    biz_sink->flush();
    
    std::cout << "自定义配置日志已写入 logs/custom/ 目录" << std::endl;
}

// 辅助函数：打印日志文件列表
void print_log_files(const std::string& directory) {
    std::cout << "\n日志文件列表 (" << directory << "):" << std::endl;
    // 这里可以添加实际的文件遍历代码
    std::cout << "  - screen_record.log" << std::endl;
    std::cout << "  - screen_record.1.log (轮转备份)" << std::endl;
    std::cout << "  - desktop_open.log" << std::endl;
    std::cout << "  - keyboard_record.log" << std::endl;
    std::cout << "  - audio_record.log" << std::endl;
}

int main() {
    try {
        std::cout << "========================================" << std::endl;
        std::cout << "   spdlog 多业务日志功能示例" << std::endl;
        std::cout << "========================================" << std::endl;
        
        // 运行各种示例
        example1_separate_loggers();
        example2_business_type_markers();
        example3_combined_sinks();
        example4_rotation_test();
        example5_multithreaded();
        example6_custom_config();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "   所有示例运行完成!" << std::endl;
        std::cout << "========================================" << std::endl;
        
        print_log_files("logs/");
        
        // 清理资源
        spdlog::shutdown();
        
        return 0;
    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "日志系统错误: " << ex.what() << std::endl;
        return 1;
    }
    catch (const std::exception& ex) {
        std::cerr << "错误: " << ex.what() << std::endl;
        return 1;
    }
}
