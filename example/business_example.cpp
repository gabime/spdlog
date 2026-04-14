// 业务日志示例程序
// 演示如何使用 business_sink 记录不同业务的日志

#include <spdlog/spdlog.h>
#include <spdlog/sinks/business_sink.h>

#include <iostream>
#include <thread>
#include <chrono>

using namespace spdlog;
using namespace spdlog::sinks;

int main() {
    try {
        // 创建日志目录
        std::string log_dir = "logs/business";
        
        // 创建业务日志 sink
        // 每个业务最多保留3个日志文件，每个文件最大30MB
        auto business_sink = std::make_shared<business_sink_mt>(
            log_dir,                    // 日志文件存放目录
            30 * 1024 * 1024,          // 每个文件最大30MB
            3                           // 最多保留3个文件
        );
        
        // 设置日志格式
        business_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        
        // 创建 logger
        auto logger = std::make_shared<spdlog::logger>("business_logger", business_sink);
        logger->set_level(spdlog::level::debug);
        
        std::cout << "=== 业务日志示例 ===" << std::endl;
        std::cout << "日志目录: " << log_dir << std::endl;
        std::cout << std::endl;
        
        // 1. 录制屏幕业务日志
        std::cout << "[1] 录制屏幕业务日志..." << std::endl;
        business_sink->set_current_business(business_type::screen_record);
        logger->info("开始录制屏幕");
        logger->info("屏幕分辨率: 1920x1080");
        logger->info("帧率: 30fps");
        logger->info("录制完成，时长: 120秒");
        
        // 2. 打开桌面业务日志
        std::cout << "[2] 打开桌面业务日志..." << std::endl;
        business_sink->set_current_business(business_type::desktop_open);
        logger->info("打开桌面");
        logger->info("桌面窗口句柄: 0x12345678");
        logger->info("桌面分辨率: 1920x1080");
        
        // 3. 录制键盘业务日志
        std::cout << "[3] 录制键盘业务日志..." << std::endl;
        business_sink->set_current_business(business_type::keyboard_record);
        logger->info("开始录制键盘");
        logger->info("捕获按键: Ctrl+C");
        logger->info("捕获按键: Ctrl+V");
        logger->info("键盘录制完成，共捕获 50 个按键事件");
        
        // 4. 录制声音业务日志
        std::cout << "[4] 录制声音业务日志..." << std::endl;
        business_sink->set_current_business(business_type::sound_record);
        logger->info("开始录制声音");
        logger->info("音频格式: PCM, 44100Hz, 16bit, 立体声");
        logger->info("录制完成，时长: 180秒");
        
        // 5. 模拟多线程场景
        std::cout << "[5] 模拟多线程场景..." << std::endl;
        std::vector<std::thread> threads;
        
        for (int i = 0; i < 4; ++i) {
            threads.emplace_back([i, logger, business_sink]() {
                business_type type = static_cast<business_type>(i % 4);
                business_sink->set_current_business(type);
                
                for (int j = 0; j < 5; ++j) {
                    logger->info("线程 {} - 业务 {} - 消息 {}", 
                        i, 
                        business_type_to_string(type),
                        j);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            });
        }
        
        for (auto &t : threads) {
            t.join();
        }
        
        // 6. 查看生成的日志文件
        std::cout << std::endl;
        std::cout << "=== 生成的日志文件 ===" << std::endl;
        std::cout << "屏幕录制日志: " << business_sink->get_business_filename(business_type::screen_record) << std::endl;
        std::cout << "桌面打开日志: " << business_sink->get_business_filename(business_type::desktop_open) << std::endl;
        std::cout << "键盘录制日志: " << business_sink->get_business_filename(business_type::keyboard_record) << std::endl;
        std::cout << "声音录制日志: " << business_sink->get_business_filename(business_type::sound_record) << std::endl;
        
        // 7. 强制刷新所有日志
        logger->flush();
        
        std::cout << std::endl;
        std::cout << "=== 示例完成 ===" << std::endl;
        std::cout << "每个业务类型的日志文件最多保留3个，每个文件不超过30MB" << std::endl;
        std::cout << "当文件超过30MB时，会自动轮转: log.txt -> log.1.txt -> log.2.txt -> log.3.txt" << std::endl;
        
    } catch (const std::exception &ex) {
        std::cerr << "错误: " << ex.what() << std::endl;
        return 1;
    }
    
    return 0;
}
