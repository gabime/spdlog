#include "test_utils.h"
#include "module_core.h"
#include "module_network.h"
#include "module_data.h"
#include <catch2/catch_all.hpp>
#include <thread>
#include <chrono>

namespace {
const std::string TEST_LOG_DIR = "test_logs";
}

TEST_CASE("Module logger isolation - separate log files", "[isolation]") {
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    module_core::CoreLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_network::NetworkLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_data::DataLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    
    module_core::CoreLogger::instance().log_info("Core module message");
    module_network::NetworkLogger::instance().log_connection("127.0.0.1", 8080);
    module_data::DataLogger::instance().log_processing_start("test_data");
    
    module_core::CoreLogger::instance().flush();
    module_network::NetworkLogger::instance().flush();
    module_data::DataLogger::instance().flush();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::string core_log = test_utils::filename_to_string(module_core::CoreLogger::instance().get_log_file());
    std::string network_log = test_utils::filename_to_string(module_network::NetworkLogger::instance().get_log_file());
    std::string data_log = test_utils::filename_to_string(module_data::DataLogger::instance().get_log_file());
    
    REQUIRE(test_utils::file_contains(core_log, "[Core]"));
    REQUIRE(test_utils::file_contains(core_log, "core_logger"));
    REQUIRE(test_utils::file_does_not_contain(core_log, "[Network]"));
    REQUIRE(test_utils::file_does_not_contain(core_log, "[Data]"));
    
    REQUIRE(test_utils::file_contains(network_log, "[Network]"));
    REQUIRE(test_utils::file_contains(network_log, "network_logger"));
    REQUIRE(test_utils::file_does_not_contain(network_log, "[Core]"));
    REQUIRE(test_utils::file_does_not_contain(network_log, "[Data]"));
    
    REQUIRE(test_utils::file_contains(data_log, "[Data]"));
    REQUIRE(test_utils::file_contains(data_log, "data_logger"));
    REQUIRE(test_utils::file_does_not_contain(data_log, "[Core]"));
    REQUIRE(test_utils::file_does_not_contain(data_log, "[Network]"));
    
    module_core::CoreLogger::instance().shutdown();
    module_network::NetworkLogger::instance().shutdown();
    module_data::DataLogger::instance().shutdown();
}

TEST_CASE("Module logger isolation - separate log levels", "[isolation]") {
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    module_core::CoreLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_network::NetworkLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_data::DataLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    
    module_core::CoreLogger::instance().set_level(spdlog::level::info);
    module_network::NetworkLogger::instance().set_level(spdlog::level::warn);
    module_data::DataLogger::instance().set_level(spdlog::level::debug);
    
    module_core::CoreLogger::instance().log_debug("Core debug message");
    module_core::CoreLogger::instance().log_info("Core info message");
    
    module_network::NetworkLogger::instance().log_connection("127.0.0.1", 8080);
    module_network::NetworkLogger::instance().log_error("Network error");
    
    module_data::DataLogger::instance().log_record_processed(1, "success");
    module_data::DataLogger::instance().log_warning("Data warning");
    
    module_core::CoreLogger::instance().flush();
    module_network::NetworkLogger::instance().flush();
    module_data::DataLogger::instance().flush();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::string core_log = test_utils::filename_to_string(module_core::CoreLogger::instance().get_log_file());
    std::string network_log = test_utils::filename_to_string(module_network::NetworkLogger::instance().get_log_file());
    std::string data_log = test_utils::filename_to_string(module_data::DataLogger::instance().get_log_file());
    
    REQUIRE(test_utils::file_does_not_contain(core_log, "Core debug message"));
    REQUIRE(test_utils::file_contains(core_log, "Core info message"));
    
    REQUIRE(test_utils::file_does_not_contain(network_log, "Connected"));
    REQUIRE(test_utils::file_contains(network_log, "Network error"));
    
    REQUIRE(test_utils::file_contains(data_log, "Record 1 processed"));
    REQUIRE(test_utils::file_contains(data_log, "Data warning"));
    
    module_core::CoreLogger::instance().shutdown();
    module_network::NetworkLogger::instance().shutdown();
    module_data::DataLogger::instance().shutdown();
}

TEST_CASE("Module logger isolation - registry separation", "[isolation]") {
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    module_core::CoreLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_network::NetworkLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_data::DataLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    
    auto core_logger = spdlog::get("core_logger");
    auto network_logger = spdlog::get("network_logger");
    auto data_logger = spdlog::get("data_logger");
    
    REQUIRE(core_logger != nullptr);
    REQUIRE(network_logger != nullptr);
    REQUIRE(data_logger != nullptr);
    
    REQUIRE(core_logger->name() == "core_logger");
    REQUIRE(network_logger->name() == "network_logger");
    REQUIRE(data_logger->name() == "data_logger");
    
    spdlog::drop("core_logger");
    REQUIRE(spdlog::get("core_logger") == nullptr);
    REQUIRE(spdlog::get("network_logger") != nullptr);
    REQUIRE(spdlog::get("data_logger") != nullptr);
    
    module_network::NetworkLogger::instance().shutdown();
    module_data::DataLogger::instance().shutdown();
}

TEST_CASE("Module logger isolation - concurrent logging", "[isolation]") {
    test_utils::prepare_logdir(TEST_LOG_DIR);
    
    module_core::CoreLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_network::NetworkLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    module_data::DataLogger::instance().init(SPDLOG_FILENAME_T(TEST_LOG_DIR));
    
    const size_t messages_per_module = 100;
    
    std::thread core_thread([&]() {
        for (size_t i = 0; i < messages_per_module; ++i) {
            module_core::CoreLogger::instance().log_info(
                "Core message " + std::to_string(i));
        }
    });
    
    std::thread network_thread([&]() {
        for (size_t i = 0; i < messages_per_module; ++i) {
            module_network::NetworkLogger::instance().log_connection(
                "192.168.1." + std::to_string(i), 8080);
        }
    });
    
    std::thread data_thread([&]() {
        for (size_t i = 0; i < messages_per_module; ++i) {
            module_data::DataLogger::instance().log_record_processed(i, "success");
        }
    });
    
    core_thread.join();
    network_thread.join();
    data_thread.join();
    
    module_core::CoreLogger::instance().flush();
    module_network::NetworkLogger::instance().flush();
    module_data::DataLogger::instance().flush();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::string core_log = test_utils::filename_to_string(module_core::CoreLogger::instance().get_log_file());
    std::string network_log = test_utils::filename_to_string(module_network::NetworkLogger::instance().get_log_file());
    std::string data_log = test_utils::filename_to_string(module_data::DataLogger::instance().get_log_file());
    
    size_t core_count = test_utils::count_occurrences(core_log, "[Core]");
    size_t network_count = test_utils::count_occurrences(network_log, "[Network]");
    size_t data_count = test_utils::count_occurrences(data_log, "[Data]");
    
    REQUIRE(core_count == messages_per_module);
    REQUIRE(network_count == messages_per_module);
    REQUIRE(data_count == messages_per_module);
    
    REQUIRE(test_utils::file_does_not_contain(core_log, "[Network]"));
    REQUIRE(test_utils::file_does_not_contain(core_log, "[Data]"));
    REQUIRE(test_utils::file_does_not_contain(network_log, "[Core]"));
    REQUIRE(test_utils::file_does_not_contain(network_log, "[Data]"));
    REQUIRE(test_utils::file_does_not_contain(data_log, "[Core]"));
    REQUIRE(test_utils::file_does_not_contain(data_log, "[Network]"));
    
    module_core::CoreLogger::instance().shutdown();
    module_network::NetworkLogger::instance().shutdown();
    module_data::DataLogger::instance().shutdown();
}
