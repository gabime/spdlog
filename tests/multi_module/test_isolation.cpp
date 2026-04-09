#include "test_utils.h"
#include "module_core.h"
#include "module_network.h"
#include "module_data.h"
#include <catch2/catch_all.hpp>
#include <thread>
#include <chrono>

namespace {
const std::string TEST_LOG_DIR = "test_logs";  ///< 测试日志目录名称
}

/**
 * @brief 测试用例：模块日志器隔离 - 独立日志文件
 * @tags [isolation]
 * 
 * 测试目的：验证三个模块的日志器是否正确地将日志写入各自独立的文件中。
 * 
 * 测试场景：
 * 1. 初始化三个模块的日志器
 * 2. 每个模块记录一条日志
 * 3. 验证每个模块的日志只出现在自己的日志文件中
 * 4. 验证其他模块的日志不会出现在本模块的日志文件中
 * 
 * 验证点：
 * - core.log 应该包含 "[Core]" 和 "core_logger"，不应该包含 "[Network]" 和 "[Data]"
 * - network.log 应该包含 "[Network]" 和 "network_logger"，不应该包含 "[Core]" 和 "[Data]"
 * - data.log 应该包含 "[Data]" 和 "data_logger"，不应该包含 "[Core]" 和 "[Network]"
 * 
 * 这是日志隔离测试的最基本验证。
 */
TEST_CASE("Module logger isolation - separate log files", "[isolation]") {
    // 准备测试环境：清除所有已存在的日志目录，创建新的空目录
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    // 初始化三个模块的日志器
    // 每个模块使用独立的日志文件
    module_core::CoreLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_network::NetworkLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_data::DataLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    
    // 每个模块记录一条日志
    // 每条日志都带有模块标识前缀
    module_core::CoreLogger::instance().log_info("Core module message");
    module_network::NetworkLogger::instance().log_connection("127.0.0.1", 8080);
    module_data::DataLogger::instance().log_processing_start("test_data");
    
    // 刷新所有日志器，确保所有日志都写入文件
    module_core::CoreLogger::instance().flush();
    module_network::NetworkLogger::instance().flush();
    module_data::DataLogger::instance().flush();
    
    // 等待异步日志处理完成（data模块使用异步日志器
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 获取各模块的日志文件路径
    // 使用 filename_to_string 转换为 std::string 以便文件操作
    std::string core_log = test_utils::filename_to_string(module_core::CoreLogger::instance().get_log_file());
    std::string network_log = test_utils::filename_to_string(module_network::NetworkLogger::instance().get_log_file());
    std::string data_log = test_utils::filename_to_string(module_data::DataLogger::instance().get_log_file());
    
    // 验证 core.log 只包含 core 模块的日志
    // 应该包含 "[Core]" 和 "core_logger"
    REQUIRE(test_utils::file_contains(core_log, "[Core]"));
    REQUIRE(test_utils::file_contains(core_log, "core_logger"));
    // 不应该包含其他模块的日志
    REQUIRE(test_utils::file_does_not_contain(core_log, "[Network]"));
    REQUIRE(test_utils::file_does_not_contain(core_log, "[Data]"));
    
    // 验证 network.log 只包含 network 模块的日志
    REQUIRE(test_utils::file_contains(network_log, "[Network]"));
    REQUIRE(test_utils::file_contains(network_log, "network_logger"));
    REQUIRE(test_utils::file_does_not_contain(network_log, "[Core]"));
    REQUIRE(test_utils::file_does_not_contain(network_log, "[Data]"));
    
    // 验证 data.log 只包含 data 模块的日志
    REQUIRE(test_utils::file_contains(data_log, "[Data]"));
    REQUIRE(test_utils::file_contains(data_log, "data_logger"));
    REQUIRE(test_utils::file_does_not_contain(data_log, "[Core]"));
    REQUIRE(test_utils::file_does_not_contain(data_log, "[Network]"));
    
    // 关闭日志器，释放资源
    module_core::CoreLogger::instance().shutdown();
    module_network::NetworkLogger::instance().shutdown();
    module_data::DataLogger::instance().shutdown();
}

/**
 * @brief 测试用例：模块日志器隔离 - 独立日志级别
 * @tags [isolation]
 * 
 * 测试目的：验证三个模块可以独立设置不同的日志级别，互不影响。
 * 
 * 测试场景：
 * 1. 初始化三个模块的日志器
 * 2. 为每个模块设置不同的日志级别
 *    - Core: INFO  INFO 级别（DEBUG 日志不会输出
 *    - Network: WARN 级别（INFO 级别日志不会输出
 *    - Data: DEBUG 级别（所有级别日志都会输出
 * 3. 每个模块记录不同级别的日志
 * 4. 验证日志级别过滤是否正确
 * 
 * 验证点：
 * - Core 模块的 DEBUG 日志不会出现在 core.log 中（因为级别设置为 INFO
 * - Network 模块的 INFO 日志不会出现在 network.log 中（因为级别设置为 WARN
 * - Data 模块的所有级别日志都会出现在 data.log 中
 * 
 * 这验证了日志级别隔离：每个模块的日志级别设置独立控制独立设置不会影响其他模块。
 */
TEST_CASE("Module logger isolation - separate log levels", "[isolation]") {
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    // 初始化三个模块的日志器
    module_core::CoreLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_network::NetworkLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_data::DataLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    
    // 为每个模块设置不同的日志级别
    // Core: INFO 级别 - DEBUG 只输出 INFO 及以上级别
    module_core::CoreLogger::instance().set_level(spdlog::level::info);
    // Network: WARN 级别 - 只输出 WARN 及以上级别
    module_network::NetworkLogger::instance().set_level(spdlog::level::warn);
    // Data: DEBUG 级别 - 输出所有级别
    module_data::DataLogger::instance().set_level(spdlog::level::debug);
    
    // Core 模块记录 DEBUG 和 INFO 级别日志
    // DEBUG 级别日志应该被过滤掉
    module_core::CoreLogger::instance().log_debug("Core debug message");
    // INFO 级别日志应该输出
    module_core::CoreLogger::instance().log_info("Core info message");
    
    // Network 模块记录 INFO 和 ERROR 级别日志
    // INFO 级别日志应该被过滤掉（因为级别设置为 WARN
    module_network::NetworkLogger::instance().log_connection("127.0.0.1", 8080);  // INFO 级别
    // ERROR 级别日志应该输出
    module_network::NetworkLogger::instance().log_error("Network error");
    
    // Data 模块记录 DEBUG 和 WARN 级别日志
    // 所有级别都应该输出
    module_data::DataLogger::instance().log_record_processed(1, "success");  // DEBUG 级别
    module_data::DataLogger::instance().log_warning("Data warning");  // WARN 级别
    
    // 刷新日志器
    module_core::CoreLogger::instance().flush();
    module_network::NetworkLogger::instance().flush();
    module_data::DataLogger::instance().flush();
    
    // 等待异步日志处理完成
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 获取日志文件路径
    std::string core_log = test_utils::filename_to_string(module_core::CoreLogger::instance().get_log_file());
    std::string network_log = test_utils::filename_to_string(module_network::NetworkLogger::instance().get_log_file());
    std::string data_log = test_utils::filename_to_string(module_data::DataLogger::instance().get_log_file());
    
    // 验证 Core 模块的日志级别过滤
    // DEBUG 级别日志不应该输出
    REQUIRE(test_utils::file_does_not_contain(core_log, "Core debug message"));
    // INFO 级别日志应该输出
    REQUIRE(test_utils::file_contains(core_log, "Core info message"));
    
    // 验证 Network 模块的日志级别过滤
    // INFO 级别日志不应该输出
    REQUIRE(test_utils::file_does_not_contain(network_log, "Connected"));
    // ERROR 级别日志应该输出
    REQUIRE(test_utils::file_contains(network_log, "Network error"));
    
    // 验证 Data 模块的日志级别过滤
    // 所有级别都应该输出
    REQUIRE(test_utils::file_contains(data_log, "Record 1 processed"));
    REQUIRE(test_utils::file_contains(data_log, "Data warning"));
    
    // 关闭日志器
    module_core::CoreLogger::instance().shutdown();
    module_network::NetworkLogger::instance().shutdown();
    module_data::DataLogger::instance().shutdown();
}

/**
 * @brief 测试用例：模块日志器隔离 - 注册表分离
 * @tags [isolation]
 * 
 * 测试目的：验证三个模块的日志器在 spdlog 全局注册表中是独立的。
 * 
 * 测试场景：
 * 1. 初始化三个模块的日志器
 * 2. 通过 spdlog::get() 从注册表获取各模块的日志器
 * 3. 验证获取的日志器名称是否正确
 * 4. 删除一个日志器，验证其他日志器不受影响
 * 
 * 验证点：
 * - 每个模块的日志器可以通过名称从注册表获取
 * - 删除 core_logger、network_logger、data_logger 是三个独立的日志器
 * - 删除 core_logger 后，network_logger 和 data_logger 仍然存在
 * 
 * 这验证了日志器在注册表中的隔离性：一个模块的日志器在注册表中是独立的实体。
 */
TEST_CASE("Module logger isolation - registry separation", "[isolation]") {
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    // 初始化三个模块的日志器
    module_core::CoreLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_network::NetworkLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_data::DataLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    
    // 从 spdlog 全局注册表获取各模块的日志器
    auto core_logger = spdlog::get("core_logger");
    auto network_logger = spdlog::get("network_logger");
    auto data_logger = spdlog::get("data_logger");
    
    // 验证获取的日志器不为空
    REQUIRE(core_logger != nullptr);
    REQUIRE(network_logger != nullptr);
    REQUIRE(data_logger != nullptr);
    
    // 验证日志器名称正确
    REQUIRE(core_logger->name() == "core_logger");
    REQUIRE(network_logger->name() == "network_logger");
    REQUIRE(data_logger->name() == "data_logger");
    
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
 * @brief 测试用例：模块日志器隔离 - 并发日志
 * @tags [isolation]
 * 
 * 测试目的：验证在多线程并发环境下，三个模块的日志器仍然保持隔离。
 * 
 * 测试场景：
 * 1. 初始化三个模块的日志器
 * 2. 创建三个线程，每个线程为一个模块记录 100 条日志
 * 3. 等待所有线程完成
 * 4. 验证每个模块的日志数量正确
 * 5. 验证日志隔离仍然隔离
 * 
 * 验证点：
 * - 每个模块的日志数量正确（100 条
 * - 每个模块的日志只出现在自己的日志文件中
 * - 其他模块的日志不会出现在本模块的日志文件中
 * 
 * 这是一个高并发场景下的日志隔离测试。
 * 验证多线程环境下日志器的线程安全性和隔离性。
 */
TEST_CASE("Module logger isolation - concurrent logging", "[isolation]") {
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    // 初始化三个模块的日志器
    module_core::CoreLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_network::NetworkLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_data::DataLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    
    // 每个模块记录 100 条日志
    const size_t messages_per_module = 100;
    
    // 创建三个线程，每个线程为一个模块记录日志
    // Core 模块线程
    std::thread core_thread([&]() {
        for (size_t i = 0; i < messages_per_module; ++i) {
            module_core::CoreLogger::instance().log_info(
                "Core message " + std::to_string(i));
        }
    });
    
    // Network 模块线程
    std::thread network_thread([&]() {
        for (size_t i = 0; i < messages_per_module; ++i) {
            module_network::NetworkLogger::instance().log_connection(
                "192.168.1." + std::to_string(i), 8080);
        }
    });
    
    // Data 模块线程
    std::thread data_thread([&]() {
        for (size_t i = 0; i < messages_per_module; ++i) {
            module_data::DataLogger::instance().log_record_processed(i, "success");
        }
    });
    
    // 等待所有线程完成
    core_thread.join();
    network_thread.join();
    data_thread.join();
    
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
    
    // 统计各模块的日志数量
    size_t core_count = test_utils::count_occurrences(core_log, "[Core]");
    size_t network_count = test_utils::count_occurrences(network_log, "[Network]");
    size_t data_count = test_utils::count_occurrences(data_log, "[Data]");
    
    // 验证日志数量正确
    REQUIRE(core_count == messages_per_module);
    REQUIRE(network_count == messages_per_module);
    REQUIRE(data_count == messages_per_module);
    
    // 验证日志隔离
    // Core 日志文件不应该包含其他模块的日志
    REQUIRE(test_utils::file_does_not_contain(core_log, "[Network]"));
    REQUIRE(test_utils::file_does_not_contain(core_log, "[Data]"));
    // Network 日志文件不应该包含其他模块的日志
    REQUIRE(test_utils::file_does_not_contain(network_log, "[Core]"));
    REQUIRE(test_utils::file_does_not_contain(network_log, "[Data]"));
    // Data 日志文件不应该包含其他模块的日志
    REQUIRE(test_utils::file_does_not_contain(data_log, "[Core]"));
    REQUIRE(test_utils::file_does_not_contain(data_log, "[Network]"));
    
    // 关闭日志器
    module_core::CoreLogger::instance().shutdown();
    module_network::NetworkLogger::instance().shutdown();
    module_data::DataLogger::instance().shutdown();
}
