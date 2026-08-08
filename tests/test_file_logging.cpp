/*
 * This content is released under the MIT License as specified in
 * https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"

#define SIMPLE_LOG "test_logs/simple_log"
#define ROTATING_LOG "test_logs/rotating_log"

TEST_CASE("simple_file_logger", "[simple_logger]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T(SIMPLE_LOG);

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("logger", filename);
    logger->set_pattern("%v");

    logger->info("Test message {}", 1);
    logger->info("Test message {}", 2);

    logger->flush();
    require_message_count(SIMPLE_LOG, 2);
    using spdlog::details::os::default_eol;
    REQUIRE(file_contents(SIMPLE_LOG) ==
            spdlog::fmt_lib::format("Test message 1{}Test message 2{}", default_eol, default_eol));
}

TEST_CASE("flush_on", "[flush_on]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T(SIMPLE_LOG);

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("logger", filename);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::info);
    logger->trace("Should not be flushed");
    REQUIRE(count_lines(SIMPLE_LOG) == 0);

    logger->info("Test message {}", 1);
    logger->info("Test message {}", 2);

    require_message_count(SIMPLE_LOG, 3);
    using spdlog::details::os::default_eol;
    REQUIRE(file_contents(SIMPLE_LOG) ==
            spdlog::fmt_lib::format("Should not be flushed{}Test message 1{}Test message 2{}",
                                    default_eol, default_eol, default_eol));
}

TEST_CASE("simple_file_logger", "[truncate]") {
    prepare_logdir();
    const spdlog::filename_t filename = SPDLOG_FILENAME_T(SIMPLE_LOG);
    const bool truncate = true;
    const auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, truncate);
    const auto logger = std::make_shared<spdlog::logger>("simple_file_logger", sink);

    logger->info("Test message {}", 3.14);
    logger->info("Test message {}", 2.71);
    logger->flush();
    REQUIRE(count_lines(SIMPLE_LOG) == 2);

    sink->truncate();
    REQUIRE(count_lines(SIMPLE_LOG) == 0);

    logger->info("Test message {}", 6.28);
    logger->flush();
    REQUIRE(count_lines(SIMPLE_LOG) == 1);
}

TEST_CASE("rotating_file_logger1", "[rotating_logger]") {
    prepare_logdir();
    size_t max_size = 1024 * 10;
    spdlog::filename_t basename = SPDLOG_FILENAME_T(ROTATING_LOG);
    auto logger = spdlog::rotating_logger_mt("logger", basename, max_size, 0);

    for (int i = 0; i < 10; ++i) {
        logger->info("Test message {}", i);
    }

    logger->flush();
    require_message_count(ROTATING_LOG, 10);
}

TEST_CASE("rotating_file_logger2", "[rotating_logger]") {
    prepare_logdir();
    size_t max_size = 1024 * 10;
    spdlog::filename_t basename = SPDLOG_FILENAME_T(ROTATING_LOG);

    {
        // make an initial logger to create the first output file
        auto logger = spdlog::rotating_logger_mt("logger", basename, max_size, 2, true);
        for (int i = 0; i < 10; ++i) {
            logger->info("Test message {}", i);
        }
        // drop causes the logger destructor to be called, which is required so the
        // next logger can rename the first output file.
        spdlog::drop(logger->name());
    }
    auto logger = spdlog::rotating_logger_mt("logger", basename, max_size, 2, true);
    for (int i = 0; i < 10; ++i) {
        logger->info("Test message {}", i);
    }
    logger->flush();
    require_message_count(ROTATING_LOG, 10);

    for (int i = 0; i < 1000; i++) {
        logger->info("Test message {}", i);
    }

    logger->flush();
    REQUIRE(get_filesize(ROTATING_LOG) <= max_size);
    REQUIRE(get_filesize(ROTATING_LOG ".1") <= max_size);
}

// test that get_current_size() tracks the file size, including across rotations
TEST_CASE("rotating_file_logger_get_current_size", "[rotating_logger]") {
    prepare_logdir();
    size_t max_size = 1024 * 10;
    spdlog::filename_t basename = SPDLOG_FILENAME_T(ROTATING_LOG);
    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(basename, max_size, 2);
    auto logger = std::make_shared<spdlog::logger>("rotating_sink_logger", sink);

    REQUIRE(sink->get_current_size() == 0);
    logger->info("Test message");
    logger->flush();
    REQUIRE(sink->get_current_size() == get_filesize(ROTATING_LOG));
    REQUIRE(sink->get_current_size() > 0);

    sink->rotate_now();
    REQUIRE(sink->get_current_size() == 0);
}

// test that a manual rotate_now() does not cause later writes to rotate the file
// prematurely, well before max_size is actually reached.
TEST_CASE("rotating_file_logger_no_premature_rotation_after_manual_rotate",
          "[rotating_logger]") {
    prepare_logdir();
    using spdlog::details::os::default_eol;
    const size_t eol_len = strlen(default_eol);
    const size_t max_size = 100;
    spdlog::filename_t basename = SPDLOG_FILENAME_T(ROTATING_LOG);
    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(basename, max_size, 2);
    auto logger = std::make_shared<spdlog::logger>("rotating_sink_logger", sink);
    logger->set_pattern("%v");

    // fill the file close to (but under) max_size, then rotate it away manually.
    const std::string msg1(89, 'a');
    logger->info(msg1);
    logger->flush();
    REQUIRE(sink->get_current_size() == msg1.size() + eol_len);

    sink->rotate_now();
    REQUIRE(get_filesize(ROTATING_LOG ".1") == msg1.size() + eol_len);

    // write two small messages whose combined real size stays well under max_size.
    const std::string msg2(4, 'b');
    logger->info(msg2);
    logger->flush();

    const std::string msg3(5, 'c');
    logger->info(msg3);
    logger->flush();

    // combined real bytes written since rotate_now() (13ish bytes) never approached
    // max_size (100), so no further rotation should have happened: ROTATING_LOG.2
    // should not exist, and ROTATING_LOG.1 should still hold only msg1.
    REQUIRE_THROWS(get_filesize(ROTATING_LOG ".2"));
    REQUIRE(get_filesize(ROTATING_LOG ".1") == msg1.size() + eol_len);
    REQUIRE(get_filesize(ROTATING_LOG) == msg2.size() + msg3.size() + 2 * eol_len);
}

// test that passing max_size=0 throws
TEST_CASE("rotating_file_logger3", "[rotating_logger]") {
    prepare_logdir();
    size_t max_size = 0;
    spdlog::filename_t basename = SPDLOG_FILENAME_T(ROTATING_LOG);
    REQUIRE_THROWS_AS(spdlog::rotating_logger_mt("logger", basename, max_size, 0),
                      spdlog::spdlog_ex);
}

// test on-demand rotation of logs
TEST_CASE("rotating_file_logger4", "[rotating_logger]") {
    prepare_logdir();
    size_t max_size = 1024 * 10;
    spdlog::filename_t basename = SPDLOG_FILENAME_T(ROTATING_LOG);
    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(basename, max_size, 2);
    auto logger = std::make_shared<spdlog::logger>("rotating_sink_logger", sink);

    logger->info("Test message - pre-rotation");
    logger->flush();

    sink->rotate_now();

    logger->info("Test message - post-rotation");
    logger->flush();

    REQUIRE(get_filesize(ROTATING_LOG) > 0);
    REQUIRE(get_filesize(ROTATING_LOG ".1") > 0);
}

// test changing the max size of the rotating file sink
TEST_CASE("rotating_file_logger5", "[rotating_logger]") {
    prepare_logdir();
    size_t max_size = 5 * 1024;
    size_t max_files = 2;
    spdlog::filename_t basename = SPDLOG_FILENAME_T(ROTATING_LOG);
    auto sink =
        std::make_shared<spdlog::sinks::rotating_file_sink_st>(basename, max_size, max_files);
    auto logger = std::make_shared<spdlog::logger>("rotating_sink_logger", sink);
    logger->set_pattern("%v");

    REQUIRE(sink->get_max_size() == max_size);
    REQUIRE(sink->get_max_files() == max_files);
    max_size = 7 * 1024;
    max_files = 3;

    sink->set_max_size(max_size);
    sink->set_max_files(max_files);
    REQUIRE(sink->get_max_size() == max_size);
    REQUIRE(sink->get_max_files() == max_files);

    const auto message = std::string(200, 'x');
    assert(message.size() < max_size);
    const auto n_messages = max_files * max_size / message.size();
    for (size_t i = 0; i < n_messages; ++i) {
        logger->info(message);
    }
    logger.reset();  // force flush and close the file

    // validate that the files were rotated correctly with the new max size and max files
    for (size_t i = 0; i <= max_files; i++) {
        // calc filenames
        // e.g. rotating_log, rotating_log.0 rotating_log.1, rotating_log.2, etc.
        std::ostringstream oss;
        oss << ROTATING_LOG;
        if (i > 0) {
            oss << '.' << i;
        }
        const auto filename = oss.str();
        const auto filesize = get_filesize(filename);
        REQUIRE(filesize <= max_size);
        if (i > 0) {
            REQUIRE(filesize >= max_size - message.size() - 2);
        }
    }
}
