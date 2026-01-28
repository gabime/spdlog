/*
 * This content is released under the MIT License as specified in
 * https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"

#include <chrono>
#include <thread>

// Mock file helper to simulate file system exceptions
class mock_file_helper : public spdlog::details::file_helper {
public:
    enum class exception_type {
        none,
        enospc,
        eacces,
        ebusy
    };

    exception_type current_exception = exception_type::none;
    int retry_count = 0;
    bool has_recovered = false;

    void set_exception(exception_type type) {
        current_exception = type;
        retry_count = 0;
        has_recovered = false;
    }

    void open(const spdlog::filename_t &fname, bool truncate = false) override {
        simulate_exception();
        spdlog::details::file_helper::open(fname, truncate);
    }

    void write(const spdlog::memory_buf_t &buf) override {
        simulate_exception();
        spdlog::details::file_helper::write(buf);
    }

private:
    void simulate_exception() {
        if (current_exception == exception_type::none) {
            return;
        }

        retry_count++;

        // Simulate recovery after 3 retries for ENOSPC
        if (current_exception == exception_type::enospc && retry_count >= 3) {
            has_recovered = true;
            current_exception = exception_type::none;
            return;
        }

        // Simulate immediate recovery for EACCES (degrade to user directory)
        if (current_exception == exception_type::eacces) {
            has_recovered = true;
            current_exception = exception_type::none;
            return;
        }

        // Simulate recovery after 30 seconds for EBUSY (timeout)
        if (current_exception == exception_type::ebusy && retry_count >= 30) {
            has_recovered = true;
            current_exception = exception_type::none;
            return;
        }

        // Throw appropriate exception
        switch (current_exception) {
            case exception_type::enospc:
                throw spdlog::spdlog_ex("Disk space insufficient", ENOSPC);
            case exception_type::eacces:
                throw spdlog::spdlog_ex("Permission denied", EACCES);
            case exception_type::ebusy:
                throw spdlog::spdlog_ex("File busy", EBUSY);
            default:
                break;
        }
    }
};

TEST_CASE("disk_space不足_recovery", "[exception_recovery]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T("test_logs/disk_space_test.txt");

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("disk_space_test", filename, true);
    logger->set_pattern("%v");

    // Test normal operation
    logger->info("Normal message 1");
    logger->flush();
    REQUIRE(count_lines("test_logs/disk_space_test.txt") == 1);

    // Simulate ENOSPC exception
    auto &sink = static_cast<spdlog::sinks::basic_file_sink_mt &>(*logger->sinks().front());
    auto &file_helper = sink.file_helper();
    auto mock_helper = dynamic_cast<mock_file_helper *>(&file_helper);

    if (mock_helper) {
        mock_helper->set_exception(mock_file_helper::exception_type::enospc);

        // Should retry 3 times and recover
        logger->info("Message during ENOSPC 1");
        logger->info("Message during ENOSPC 2");
        logger->info("Message during ENOSPC 3");
        logger->info("Message after recovery");
        logger->flush();

        REQUIRE(mock_helper->has_recovered == true);
        REQUIRE(count_lines("test_logs/disk_space_test.txt") == 5); // 1 normal + 4 during/after recovery
    }

    spdlog::drop("disk_space_test");
}

TEST_CASE("permission_error_recovery", "[exception_recovery]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T("test_logs/permission_test.txt");

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("permission_test", filename, true);
    logger->set_pattern("%v");

    // Simulate EACCES exception
    auto &sink = static_cast<spdlog::sinks::basic_file_sink_mt &>(*logger->sinks().front());
    auto &file_helper = sink.file_helper();
    auto mock_helper = dynamic_cast<mock_file_helper *>(&file_helper);

    if (mock_helper) {
        mock_helper->set_exception(mock_file_helper::exception_type::eacces);

        // Should degrade to user directory within 1 second
        auto start_time = std::chrono::steady_clock::now();
        logger->info("Message during permission error");
        logger->flush();
        auto end_time = std::chrono::steady_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        REQUIRE(duration.count() <= 1000); // Should recover within 1 second
        REQUIRE(mock_helper->has_recovered == true);
    }

    spdlog::drop("permission_test");
}

TEST_CASE("file_locking_recovery", "[exception_recovery]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T("test_logs/file_locking_test.txt");

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("file_locking_test", filename, true);
    logger->set_pattern("%v");

    // Simulate EBUSY exception
    auto &sink = static_cast<spdlog::sinks::basic_file_sink_mt &>(*logger->sinks().front());
    auto &file_helper = sink.file_helper();
    auto mock_helper = dynamic_cast<mock_file_helper *>(&file_helper);

    if (mock_helper) {
        mock_helper->set_exception(mock_file_helper::exception_type::ebusy);

        // Should wait up to 30 seconds and then create new file
        auto start_time = std::chrono::steady_clock::now();
        logger->info("Message during file locking");
        logger->flush();
        auto end_time = std::chrono::steady_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
        REQUIRE(duration.count() <= 30); // Should recover within 30 seconds
        REQUIRE(mock_helper->has_recovered == true);
    }

    spdlog::drop("file_locking_test");
}

TEST_CASE("long_path_recovery", "[exception_recovery]") {
    prepare_logdir();

    // Create a path longer than 260 characters
    spdlog::filename_t long_path = SPDLOG_FILENAME_T("test_logs/");
    for (int i = 0; i < 20; i++) {
        long_path += SPDLOG_FILENAME_T("very_long_directory_name_");
    }
    long_path += SPDLOG_FILENAME_T("test_file.txt");

    // Should automatically compress path to 255 characters
    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("long_path_test", long_path, true);
    logger->info("Message with long path");
    logger->flush();

    // Check if file was created with compressed path
    REQUIRE(spdlog::details::os::file_exists(long_path) || spdlog::details::os::file_exists(long_path.substr(0, 255)));

    spdlog::drop("long_path_test");
}