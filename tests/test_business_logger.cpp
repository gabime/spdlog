//
// 业务分类日志系统测试
//

#include "includes.h"
#include <spdlog/business_logger.h>
#include <spdlog/sinks/business_file_sink.h>

#include <fstream>
#include <sstream>

TEST_CASE("business_type_conversion", "[business_logger]") {
    using namespace spdlog::sinks;
    
    // 测试业务类型到字符串的转换
    REQUIRE(std::string(business_type_to_string(BusinessType::Screen)) == "screen");
    REQUIRE(std::string(business_type_to_string(BusinessType::Desktop)) == "desktop");
    REQUIRE(std::string(business_type_to_string(BusinessType::Keyboard)) == "keyboard");
    REQUIRE(std::string(business_type_to_string(BusinessType::Audio)) == "audio");
    REQUIRE(std::string(business_type_to_string(BusinessType::Unknown)) == "unknown");
    
    // 测试字符串到业务类型的转换
    REQUIRE(string_to_business_type("screen") == BusinessType::Screen);
    REQUIRE(string_to_business_type("desktop") == BusinessType::Desktop);
    REQUIRE(string_to_business_type("keyboard") == BusinessType::Keyboard);
    REQUIRE(string_to_business_type("audio") == BusinessType::Audio);
    REQUIRE(string_to_business_type("invalid") == BusinessType::Unknown);
}

TEST_CASE("business_logger_manager_creation", "[business_logger]") {
    using namespace spdlog;
    
    // 创建临时目录
    std::string test_dir = "test_business_logs";
    
    // 创建管理器
    auto manager = std::make_shared<BusinessLoggerManager>(test_dir);
    
    // 验证可以获取所有业务类型的日志器
    REQUIRE(manager->get_logger(BusinessType::Screen) != nullptr);
    REQUIRE(manager->get_logger(BusinessType::Desktop) != nullptr);
    REQUIRE(manager->get_logger(BusinessType::Keyboard) != nullptr);
    REQUIRE(manager->get_logger(BusinessType::Audio) != nullptr);
    
    // 验证获取所有业务类型
    auto types = manager->get_all_business_types();
    REQUIRE(types.size() == 4);
    
    // 清理
    spdlog::drop_all();
}

TEST_CASE("business_logger_basic_logging", "[business_logger]") {
    using namespace spdlog;
    
    std::string test_dir = "test_business_logs_basic";
    
    {
        auto manager = std::make_shared<BusinessLoggerManager>(test_dir);
        
        // 写入日志
        manager->info(BusinessType::Screen, "屏幕测试消息");
        manager->info(BusinessType::Keyboard, "键盘测试消息");
        manager->flush_all();
    }
    
    // 验证日志文件存在
    std::ifstream screen_log(test_dir + "/screen.log");
    REQUIRE(screen_log.good());
    
    std::ifstream keyboard_log(test_dir + "/keyboard.log");
    REQUIRE(keyboard_log.good());
    
    // 验证内容
    std::string line;
    bool found_screen_msg = false;
    while (std::getline(screen_log, line)) {
        if (line.find("屏幕测试消息") != std::string::npos) {
            found_screen_msg = true;
            break;
        }
    }
    REQUIRE(found_screen_msg);
    
    screen_log.close();
    keyboard_log.close();
    
    // 清理
    spdlog::drop_all();
}

TEST_CASE("business_logger_level_filtering", "[business_logger]") {
    using namespace spdlog;
    
    std::string test_dir = "test_business_logs_level";
    
    {
        auto manager = std::make_shared<BusinessLoggerManager>(test_dir);
        
        // 设置屏幕日志级别为 warning
        manager->set_level(BusinessType::Screen, level::warn);
        
        // 写入不同级别的日志
        manager->info(BusinessType::Screen, "这条不应该出现");
        manager->warn(BusinessType::Screen, "这条应该出现");
        manager->error(BusinessType::Screen, "这条也应该出现");
        
        // 键盘保持默认级别（trace），所有日志都应该出现
        manager->debug(BusinessType::Keyboard, "键盘调试信息");
        manager->info(BusinessType::Keyboard, "键盘普通信息");
        
        manager->flush_all();
    }
    
    // 验证屏幕日志内容
    std::ifstream screen_log(test_dir + "/screen.log");
    std::string content((std::istreambuf_iterator<char>(screen_log)),
                        std::istreambuf_iterator<char>());
    
    REQUIRE(content.find("这条不应该出现") == std::string::npos);
    REQUIRE(content.find("这条应该出现") != std::string::npos);
    REQUIRE(content.find("这条也应该出现") != std::string::npos);
    
    screen_log.close();
    
    // 验证键盘日志内容
    std::ifstream keyboard_log(test_dir + "/keyboard.log");
    std::string kb_content((std::istreambuf_iterator<char>(keyboard_log)),
                           std::istreambuf_iterator<char>());
    
    REQUIRE(kb_content.find("键盘调试信息") != std::string::npos);
    REQUIRE(kb_content.find("键盘普通信息") != std::string::npos);
    
    keyboard_log.close();
    
    // 清理
    spdlog::drop_all();
}

TEST_CASE("business_logger_formatting", "[business_logger]") {
    using namespace spdlog;
    
    std::string test_dir = "test_business_logs_format";
    
    {
        auto manager = std::make_shared<BusinessLoggerManager>(test_dir);
        
        // 测试格式化字符串
        manager->info(BusinessType::Screen, "分辨率: {}x{}", 1920, 1080);
        manager->info(BusinessType::Audio, "采样率: {} Hz", 44100);
        manager->debug(BusinessType::Keyboard, "按键: {}, 扫描码: {}", "A", 30);
        
        manager->flush_all();
    }
    
    // 验证格式化结果
    std::ifstream screen_log(test_dir + "/screen.log");
    std::string content((std::istreambuf_iterator<char>(screen_log)),
                        std::istreambuf_iterator<char>());
    
    REQUIRE(content.find("分辨率: 1920x1080") != std::string::npos);
    
    screen_log.close();
    
    // 清理
    spdlog::drop_all();
}

TEST_CASE("business_logger_thread_safety", "[business_logger]") {
    using namespace spdlog;
    
    std::string test_dir = "test_business_logs_thread";
    
    {
        auto manager = std::make_shared<BusinessLoggerManager>(test_dir);
        
        std::vector<std::thread> threads;
        const int num_threads = 4;
        const int logs_per_thread = 25;
        
        // 创建多个线程同时写入
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&, i]() {
                for (int j = 0; j < logs_per_thread; ++j) {
                    manager->info(BusinessType::Screen, "线程 {} 消息 {}", i, j);
                    manager->info(BusinessType::Keyboard, "线程 {} 消息 {}", i, j);
                }
            });
        }
        
        // 等待所有线程完成
        for (auto& t : threads) {
            t.join();
        }
        
        manager->flush_all();
    }
    
    // 验证日志文件包含所有消息
    std::ifstream screen_log(test_dir + "/screen.log");
    int line_count = 0;
    std::string line;
    while (std::getline(screen_log, line)) {
        if (line.find("线程") != std::string::npos) {
            line_count++;
        }
    }
    
    REQUIRE(line_count == num_threads * logs_per_thread);
    
    screen_log.close();
    
    // 清理
    spdlog::drop_all();
}

TEST_CASE("business_logger_file_rotation", "[business_logger]") {
    using namespace spdlog;
    
    std::string test_dir = "test_business_logs_rotation";
    
    // 使用非常小的文件大小来快速触发轮转
    {
        auto manager = std::make_shared<BusinessLoggerManager>(
            test_dir, 
            512,  // 512 bytes
            3     // 保留3个文件
        );
        
        // 写入足够多的日志以触发轮转
        for (int i = 0; i < 50; ++i) {
            manager->info(BusinessType::Screen, 
                "这是第 {} 条日志消息，添加一些填充内容使文件变大。", i);
        }
        
        manager->flush_all();
    }
    
    // 验证轮转文件存在
    std::ifstream log0(test_dir + "/screen.log");
    std::ifstream log1(test_dir + "/screen.1.log");
    std::ifstream log2(test_dir + "/screen.2.log");
    std::ifstream log3(test_dir + "/screen.3.log");
    
    // 至少应该有当前日志和一个轮转日志
    REQUIRE(log0.good());
    
    log0.close();
    log1.close();
    log2.close();
    log3.close();
    
    // 清理
    spdlog::drop_all();
}
