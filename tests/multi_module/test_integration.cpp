#include "test_utils.h"
#include "module_core.h"
#include "module_network.h"
#include "module_data.h"
#include <catch2/catch_all.hpp>
#include <thread>
#include <chrono>
#include <vector>

namespace {
const std::string TEST_LOG_DIR = "test_logs";
}

TEST_CASE("Integration test - all modules working together", "[integration]") {
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    module_core::CoreLogger::instance().init(TEST_LOG_DIR);
    module_network::NetworkLogger::instance().init(TEST_LOG_DIR);
    module_data::DataLogger::instance().init(TEST_LOG_DIR);
    
    module_core::CoreLogger::instance().log_info("Application starting");
    
    module_network::NetworkSimulator net_sim;
    net_sim.connect("database.example.com", 5432);
    net_sim.connect("api.example.com", 443);
    
    module_core::CoreLogger::instance().log_info("Connected to all services");
    
    module_data::DataProcessor data_processor;
    data_processor.load_dataset("user_data", 1000);
    data_processor.simulate_error_on_record(500);
    data_processor.simulate_warning_on_record(250);
    data_processor.process_all_records();
    
    net_sim.send_data("api.example.com", "Processed 1000 records");
    net_sim.disconnect("database.example.com", 5432);
    net_sim.disconnect("api.example.com", 443);
    
    module_core::CoreLogger::instance().log_info("Application shutting down");
    
    module_core::CoreLogger::instance().flush();
    module_network::NetworkLogger::instance().flush();
    module_data::DataLogger::instance().flush();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::string core_log = module_core::CoreLogger::instance().get_log_file();
    std::string network_log = module_network::NetworkLogger::instance().get_log_file();
    std::string data_log = module_data::DataLogger::instance().get_log_file();
    
    REQUIRE(test_utils::file_contains(core_log, "Application starting"));
    REQUIRE(test_utils::file_contains(core_log, "Connected to all services"));
    REQUIRE(test_utils::file_contains(core_log, "Application shutting down"));
    
    REQUIRE(test_utils::file_contains(network_log, "database.example.com"));
    REQUIRE(test_utils::file_contains(network_log, "api.example.com"));
    REQUIRE(test_utils::count_occurrences(network_log, "Connected") == 2);
    REQUIRE(test_utils::count_occurrences(network_log, "Disconnected") == 2);
    
    REQUIRE(test_utils::file_contains(data_log, "user_data"));
    REQUIRE(test_utils::file_contains(data_log, "Finished processing"));
    REQUIRE(test_utils::file_contains(data_log, "Error processing record 500"));
    REQUIRE(test_utils::file_contains(data_log, "Record 250 has anomalies"));
    
    module_core::CoreLogger::instance().shutdown();
    module_network::NetworkLogger::instance().shutdown();
    module_data::DataLogger::instance().shutdown();
}

TEST_CASE("Integration test - module independence", "[integration]") {
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    SECTION("Only core module") {
        module_core::CoreLogger::instance().init(TEST_LOG_DIR);
        module_core::CoreLogger::instance().log_info("Core only test");
        module_core::CoreLogger::instance().flush();
        
        std::string core_log = module_core::CoreLogger::instance().get_log_file();
        REQUIRE(test_utils::file_contains(core_log, "Core only test"));
        
        module_core::CoreLogger::instance().shutdown();
    }
    
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    SECTION("Only network module") {
        module_network::NetworkLogger::instance().init(TEST_LOG_DIR);
        module_network::NetworkSimulator net_sim;
        net_sim.connect("localhost", 8080);
        module_network::NetworkLogger::instance().flush();
        
        std::string network_log = module_network::NetworkLogger::instance().get_log_file();
        REQUIRE(test_utils::file_contains(network_log, "localhost"));
        
        module_network::NetworkLogger::instance().shutdown();
    }
    
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    SECTION("Only data module") {
        module_data::DataLogger::instance().init(TEST_LOG_DIR);
        module_data::DataProcessor data_processor;
        data_processor.load_dataset("small_data", 10);
        data_processor.process_all_records();
        module_data::DataLogger::instance().flush();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::string data_log = module_data::DataLogger::instance().get_log_file();
        REQUIRE(test_utils::file_contains(data_log, "small_data"));
        
        module_data::DataLogger::instance().shutdown();
    }
}

TEST_CASE("Integration test - logger retrieval across modules", "[integration]") {
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    module_core::CoreLogger::instance().init(TEST_LOG_DIR);
    module_network::NetworkLogger::instance().init(TEST_LOG_DIR);
    module_data::DataLogger::instance().init(TEST_LOG_DIR);
    
    auto core_from_module = module_core::CoreLogger::instance().get_logger();
    auto core_from_registry = spdlog::get("core_logger");
    REQUIRE(core_from_module == core_from_registry);
    
    auto network_from_module = module_network::NetworkLogger::instance().get_logger();
    auto network_from_registry = spdlog::get("network_logger");
    REQUIRE(network_from_module == network_from_registry);
    
    auto data_from_module = module_data::DataLogger::instance().get_logger();
    auto data_from_registry = spdlog::get("data_logger");
    REQUIRE(data_from_module == data_from_registry);
    
    REQUIRE(core_from_module != network_from_module);
    REQUIRE(core_from_module != data_from_module);
    REQUIRE(network_from_module != data_from_module);
    
    module_core::CoreLogger::instance().shutdown();
    module_network::NetworkLogger::instance().shutdown();
    module_data::DataLogger::instance().shutdown();
}

TEST_CASE("Integration test - async and sync logging together", "[integration]") {
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    module_core::CoreLogger::instance().init(TEST_LOG_DIR);
    module_data::DataLogger::instance().init(TEST_LOG_DIR, 1024, 2);
    
    const size_t sync_messages = 100;
    const size_t async_messages = 500;
    
    for (size_t i = 0; i < sync_messages; ++i) {
        module_core::CoreLogger::instance().log_info(
            "Sync message " + std::to_string(i));
    }
    
    std::thread async_thread([&]() {
        for (size_t i = 0; i < async_messages; ++i) {
            module_data::DataLogger::instance().log_record_processed(i, "success");
        }
    });
    
    async_thread.join();
    
    module_core::CoreLogger::instance().flush();
    module_data::DataLogger::instance().flush();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::string core_log = module_core::CoreLogger::instance().get_log_file();
    std::string data_log = module_data::DataLogger::instance().get_log_file();
    
    size_t sync_count = test_utils::count_occurrences(core_log, "[Core]");
    size_t async_count = test_utils::count_occurrences(data_log, "[Data]");
    
    REQUIRE(sync_count == sync_messages);
    REQUIRE(async_count == async_messages);
    
    REQUIRE(module_data::DataLogger::instance().get_overrun_counter() == 0);
    REQUIRE(module_data::DataLogger::instance().get_discard_counter() == 0);
    
    module_core::CoreLogger::instance().shutdown();
    module_data::DataLogger::instance().shutdown();
}
