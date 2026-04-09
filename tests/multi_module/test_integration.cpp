#include "test_utils.h"
#include "module_core.h"
#include "module_network.h"
#include "module_data.h"
#include <catch2/catch_all.hpp>
#include <thread>
#include <chrono>
#include <vector>

namespace {
const std::string TEST_LOG_DIR = "test_logs";  ///< 测试日志目录名称
}

/**
 * @brief 测试用例：集成测试 - 所有模块协同工作
 * @tags [integration]
 * 
 * 测试目的：验证三个模块在实际应用场景中协同工作时的日志隔离性。
 * 
 * 测试场景：
 * 模拟一个完整的应用程序流程：
 * 1. 应用程序启动（Core 模块记录日志
 * 2. 连接到外部服务（Network 模块记录日志
 * 3. 处理数据（Data 模块记录日志
 * 4. 断开连接（Network 模块记录日志
 * 5. 应用程序关闭（Core 模块记录日志
 * 
 * 验证点：
 * - Core 模块的日志只出现在 core.log 中
 * - Network 模块的日志只出现在 network.log 中
 * - Data 模块的日志只出现在 data.log 中
 * - 各模块的日志数量正确
 * 
 * 这是一个端到端的集成测试，验证多模块环境下的日志隔离。
 */
TEST_CASE("Integration test - all modules working together", "[integration]") {
    // 准备测试环境
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    // 初始化三个模块的日志器
    module_core::CoreLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_network::NetworkLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_data::DataLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    
    // 模拟应用程序启动
    module_core::CoreLogger::instance().log_info("Application starting");
    
    // 模拟连接到外部服务
    module_network::NetworkSimulator net_sim;
    net_sim.connect("database.example.com", 5432);  // 连接数据库
    net_sim.connect("api.example.com", 443);         // 连接 API 服务
    
    // 记录连接完成
    module_core::CoreLogger::instance().log_info("Connected to all services");
    
    // 模拟数据处理
    module_data::DataProcessor data_processor;
    data_processor.load_dataset("user_data", 1000);      // 加载数据集
    data_processor.simulate_error_on_record(500);         // 模拟记录 500 处理错误
    data_processor.simulate_warning_on_record(250);       // 模拟记录 250 有警告
    data_processor.process_all_records();                  // 处理所有记录
    
    // 模拟发送处理结果
    net_sim.send_data("api.example.com", "Processed 1000 records");
    
    // 模拟断开连接
    net_sim.disconnect("database.example.com", 5432);
    net_sim.disconnect("api.example.com", 443);
    
    // 模拟应用程序关闭
    module_core::CoreLogger::instance().log_info("Application shutting down");
    
    // 刷新所有日志器
    module_core::CoreLogger::instance().flush();
    module_network::NetworkLogger::instance().flush();
    module_data::DataLogger::instance().flush();
    
    // 等待异步日志处理完成
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // 获取日志文件路径
    std::string core_log = test_utils::filename_to_string(module_core::CoreLogger::instance().get_log_file());
    std::string network_log = test_utils::filename_to_string(module_network::NetworkLogger::instance().get_log_file());
    std::string data_log = test_utils::filename_to_string(module_data::DataLogger::instance().get_log_file());
    
    // 验证 Core 模块的日志
    // 应该包含应用程序启动和关闭的日志
    REQUIRE(test_utils::file_contains(core_log, "Application starting"));
    REQUIRE(test_utils::file_contains(core_log, "Connected to all services"));
    REQUIRE(test_utils::file_contains(core_log, "Application shutting down"));
    
    // 验证 Network 模块的日志
    // 应该包含连接和断开连接的日志
    REQUIRE(test_utils::file_contains(network_log, "database.example.com"));
    REQUIRE(test_utils::file_contains(network_log, "api.example.com"));
    // 应该有 2 次连接和 2 次断开连接
    REQUIRE(test_utils::count_occurrences(network_log, "Connected") == 2);
    REQUIRE(test_utils::count_occurrences(network_log, "Disconnected") == 2);
    
    // 验证 Data 模块的日志
    // 应该包含数据处理的日志
    REQUIRE(test_utils::file_contains(data_log, "user_data"));
    REQUIRE(test_utils::file_contains(data_log, "Finished processing"));
    // 应该包含错误和警告的日志
    REQUIRE(test_utils::file_contains(data_log, "Error processing record 500"));
    REQUIRE(test_utils::file_contains(data_log, "Record 250 has anomalies"));
    
    // 关闭日志器
    module_core::CoreLogger::instance().shutdown();
    module_network::NetworkLogger::instance().shutdown();
    module_data::DataLogger::instance().shutdown();
}

/**
 * @brief 测试用例：集成测试 - 模块独立性
 * @tags [integration]
 * 
 * 测试目的：验证每个模块可以独立工作，不依赖其他模块。
 * 
 * 测试场景：
 * 1. 只使用 Core 模块
 * 2. 只使用 Network 模块
 * 3. 只使用 Data 模块
 * 
 * 验证点：
 * - 每个模块可以独立初始化和使用
 * - 每个模块的日志正确写入自己的日志文件
 * - 模块之间没有相互依赖
 * 
 * 这验证了模块的独立性：每个模块都是自包含的，可以单独使用。
 */
TEST_CASE("Integration test - module independence", "[integration]") {
    // SECTION 1: 只使用 Core 模块
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    SECTION("Only core module") {
        // 只初始化 Core 模块
        module_core::CoreLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
        module_core::CoreLogger::instance().log_info("Core only test");
        module_core::CoreLogger::instance().flush();
        
        // 验证 Core 模块的日志正确写入
        std::string core_log = test_utils::filename_to_string(module_core::CoreLogger::instance().get_log_file());
        REQUIRE(test_utils::file_contains(core_log, "Core only test"));
        
        module_core::CoreLogger::instance().shutdown();
    }
    
    // 准备新的测试环境
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    // SECTION 2: 只使用 Network 模块
    SECTION("Only network module") {
        // 只初始化 Network 模块
        module_network::NetworkLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
        module_network::NetworkSimulator net_sim;
        net_sim.connect("localhost", 8080);
        module_network::NetworkLogger::instance().flush();
        
        // 验证 Network 模块的日志正确写入
        std::string network_log = test_utils::filename_to_string(module_network::NetworkLogger::instance().get_log_file());
        REQUIRE(test_utils::file_contains(network_log, "localhost"));
        
        module_network::NetworkLogger::instance().shutdown();
    }
    
    // 准备新的测试环境
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    // SECTION 3: 只使用 Data 模块
    SECTION("Only data module") {
        // 只初始化 Data 模块
        module_data::DataLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
        module_data::DataProcessor data_processor;
        data_processor.load_dataset("small_data", 10);
        data_processor.process_all_records();
        module_data::DataLogger::instance().flush();
        
        // 等待异步日志处理完成
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 验证 Data 模块的日志正确写入
        std::string data_log = test_utils::filename_to_string(module_data::DataLogger::instance().get_log_file());
        REQUIRE(test_utils::file_contains(data_log, "small_data"));
        
        module_data::DataLogger::instance().shutdown();
    }
}

/**
 * @brief 测试用例：集成测试 - 跨模块日志器检索
 * @tags [integration]
 * 
 * 测试目的：验证可以从 spdlog 全局注册表中检索各模块的日志器。
 * 
 * 测试场景：
 * 1. 初始化三个模块的日志器
 * 2. 从模块内部获取日志器
 * 3. 从 spdlog 注册表获取日志器
 * 4. 验证两种方式获取的是同一个日志器
 * 5. 删除一个日志器，验证其他日志器不受影响
 * 
 * 验证点：
 * - 模块内部获取的日志器与注册表获取的是同一个实例
 * - 三个日志器是不同的实例
 * - 删除一个日志器不影响其他日志器
 * 
 * 这验证了日志器在注册表中的管理和隔离。
 */
TEST_CASE("Integration test - logger retrieval across modules", "[integration]") {
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    // 初始化三个模块的日志器
    module_core::CoreLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_network::NetworkLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_data::DataLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    
    // 从模块内部获取日志器
    auto core_from_module = module_core::CoreLogger::instance().get_logger();
    // 从 spdlog 注册表获取日志器
    auto core_from_registry = spdlog::get("core_logger");
    // 验证是同一个实例
    REQUIRE(core_from_module == core_from_registry);
    
    // 验证 Network 模块
    auto network_from_module = module_network::NetworkLogger::instance().get_logger();
    auto network_from_registry = spdlog::get("network_logger");
    REQUIRE(network_from_module == network_from_registry);
    
    // 验证 Data 模块
    auto data_from_module = module_data::DataLogger::instance().get_logger();
    auto data_from_registry = spdlog::get("data_logger");
    REQUIRE(data_from_module == data_from_registry);
    
    // 验证三个日志器是不同的实例
    REQUIRE(core_from_module != network_from_module);
    REQUIRE(core_from_module != data_from_module);
    REQUIRE(network_from_module != data_from_module);
    
    // 删除 core_logger 从注册表中移除
    spdlog::drop("core_logger");
    
    // 验证 core_logger 已被删除
    REQUIRE(spdlog::get("core_logger") == nullptr);
    // 验证其他日志器仍然存在
    REQUIRE(spdlog::get("network_logger") != nullptr);
    REQUIRE(spdlog::get("data_logger") != nullptr);
    
    // 关闭剩余的日志器
    module_network::NetworkLogger::instance().shutdown();
    module_data::DataLogger::instance().shutdown();
}

/**
 * @brief 测试用例：集成测试 - 同步和异步日志协同工作
 * @tags [integration]
 * 
 * 测试目的：验证同步日志器和异步日志器可以协同工作。
 * 
 * 测试场景：
 * 1. 初始化 Core 模块（同步日志器）
 * 2. 初始化 Data 模块（异步日志器，使用 2 个后台线程
 * 3. Core 模块同步记录 100 条日志
 * 4. Data 模块异步记录 500 条日志
 * 5. 验证两种日志模式都正确工作
 * 
 * 验证点：
 * - 同步日志器的日志数量正确
 * - 异步日志器的日志数量正确
 * - 异步日志器没有消息丢失（overrun_counter 和 discard_counter 为 0
 * - 两种日志模式互不干扰
 * 
 * 这验证了同步和异步日志器可以在同一个应用程序中协同工作。
 */
TEST_CASE("Integration test - async and sync logging together", "[integration]") {
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    // 初始化 Core 模块（同步日志器）
    module_core::CoreLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    // 初始化 Data 模块（异步日志器，使用 2 个后台线程
    module_data::DataLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR), 1024, 2);
    
    // 日志数量配置
    const size_t sync_messages = 100;   // 同步日志数量
    const size_t async_messages = 500;  // 异步日志数量
    
    // Core 模块同步记录日志
    for (size_t i = 0; i < sync_messages; ++i) {
        module_core::CoreLogger::instance().log_info(
            "Sync message " + std::to_string(i));
    }
    
    // Data 模块异步记录日志（在单独的线程中
    std::thread async_thread([&]() {
        for (size_t i = 0; i < async_messages; ++i) {
            module_data::DataLogger::instance().log_record_processed(i, "success");
        }
    });
    
    // 等待异步线程完成
    async_thread.join();
    
    // 刷新所有日志器
    module_core::CoreLogger::instance().flush();
    module_data::DataLogger::instance().flush();
    
    // 等待异步日志处理完成
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // 获取日志文件路径
    std::string core_log = test_utils::filename_to_string(module_core::CoreLogger::instance().get_log_file());
    std::string data_log = test_utils::filename_to_string(module_data::DataLogger::instance().get_log_file());
    
    // 统计日志数量
    size_t sync_count = test_utils::count_occurrences(core_log, "[Core]");
    size_t async_count = test_utils::count_occurrences(data_log, "[Data]");
    
    // 验证日志数量正确
    REQUIRE(sync_count == sync_messages);
    REQUIRE(async_count == async_messages);
    
    // 验证异步日志器没有消息丢失
    REQUIRE(module_data::DataLogger::instance().get_overrun_counter() == 0);
    REQUIRE(module_data::DataLogger::instance().get_discard_counter() == 0);
    
    // 关闭日志器
    module_core::CoreLogger::instance().shutdown();
    module_data::DataLogger::instance().shutdown();
}
