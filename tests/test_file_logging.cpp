/*
 * This content is released under the MIT License as specified in
 * https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"

#define SIMPLE_LOG "test_logs/simple_log"
#define ROTATING_LOG "test_logs/rotating_log"
#define ROTATING_DATETIME_LOG "test_logs/rotating_datetime_log"

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
#ifndef SPDLOG_NO_EXCEPTIONS
TEST_CASE("rotating_file_logger3", "[rotating_logger]") {
    prepare_logdir();
    size_t max_size = 0;
    spdlog::filename_t basename = SPDLOG_FILENAME_T(ROTATING_LOG);
    REQUIRE_THROWS_AS(spdlog::rotating_logger_mt("logger", basename, max_size, 0),
                      spdlog::spdlog_ex);
}
#endif  // SPDLOG_NO_EXCEPTIONS

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

TEST_CASE("custom_buffer_size_basic", "[file_helper]") {
    prepare_logdir();
    spdlog::filename_t filename = SPDLOG_FILENAME_T(SIMPLE_LOG);
    size_t custom_buf_size = 64 * 1024;

    auto logger = spdlog::basic_logger_mt("custom_buf_logger", filename, false, {}, custom_buf_size);
    logger->set_pattern("%v");

    auto sink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_mt>(logger->sinks().front());
    REQUIRE(sink != nullptr);
    REQUIRE(sink->buffer_size() == custom_buf_size);

    logger->info("Buffered message 1");
    logger->info("Buffered message 2");
    logger->flush();

    require_message_count(SIMPLE_LOG, 2);
    using spdlog::details::os::default_eol;
    REQUIRE(file_contents(SIMPLE_LOG) ==
            spdlog::fmt_lib::format("Buffered message 1{}Buffered message 2{}", default_eol, default_eol));

    sink->set_buffer_size(128 * 1024);
    REQUIRE(sink->buffer_size() == 128 * 1024);
}

TEST_CASE("custom_buffer_size_rotating", "[rotating_logger]") {
    prepare_logdir();
    spdlog::filename_t basename = SPDLOG_FILENAME_T(ROTATING_LOG);
    size_t custom_buf_size = 32 * 1024;

    auto logger = spdlog::rotating_logger_mt("custom_buf_rotating", basename, 1024 * 1024, 2, false, {}, custom_buf_size);
    logger->set_pattern("%v");

    auto sink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_mt>(logger->sinks().front());
    REQUIRE(sink != nullptr);
    REQUIRE(sink->buffer_size() == custom_buf_size);

    logger->info("Rotating buffered message");
    logger->flush();

    REQUIRE(get_filesize(ROTATING_LOG) > 0);
}

TEST_CASE("custom_buffer_size_daily", "[daily_logger]") {
    prepare_logdir();
    spdlog::filename_t basename = SPDLOG_FILENAME_T("test_logs/daily_buf_log.txt");
    size_t custom_buf_size = 16 * 1024;

    auto logger = spdlog::daily_logger_mt("custom_buf_daily", basename, 23, 59, false, 0, {}, custom_buf_size);
    logger->set_pattern("%v");

    auto sink = std::dynamic_pointer_cast<spdlog::sinks::daily_file_sink_mt>(logger->sinks().front());
    REQUIRE(sink != nullptr);
    REQUIRE(sink->buffer_size() == custom_buf_size);

    logger->info("Daily buffered message");
    logger->flush();

    auto actual_filename = spdlog::details::os::filename_to_str(sink->filename());
    REQUIRE(get_filesize(actual_filename) > 0);
}

TEST_CASE("custom_buffer_size_preserves_pending_output", "[file_helper]") {
    prepare_logdir();
    const size_t initial_size = GENERATE(size_t{0}, size_t{64 * 1024});
    const size_t updated_size =
        GENERATE(size_t{0}, size_t{32 * 1024}, size_t{64 * 1024}, size_t{128 * 1024});
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(
        SPDLOG_FILENAME_T("test_logs/buffer_resize.txt"), true, spdlog::file_event_handlers{},
        initial_size);
    spdlog::logger logger("buffer_resize", sink);
    logger.set_pattern("%v");

    logger.info("before resize");
    sink->set_buffer_size(updated_size);
    REQUIRE(sink->buffer_size() == updated_size);
    logger.info("after resize");
    logger.flush();

    REQUIRE(file_contents("test_logs/buffer_resize.txt") ==
            spdlog::fmt_lib::format("before resize{}after resize{}", spdlog::details::os::default_eol,
                                   spdlog::details::os::default_eol));
}

TEST_CASE("custom_buffer_size_rotating_tracks_reopen_callbacks", "[rotating_logger]") {
    prepare_logdir();
    spdlog::file_event_handlers handlers;
    int opens = 0;
    handlers.after_open = [&](spdlog::filename_t, std::FILE *stream) {
        std::fputs("header\n", stream);
        if (++opens == 1) {
            std::fflush(stream);
        }
    };
    handlers.before_close = [](spdlog::filename_t, std::FILE *stream) {
        std::fputs("footer\n", stream);
    };
    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(
        SPDLOG_FILENAME_T("test_logs/buffer_resize.txt"), 1024, 2, false, handlers, 64 * 1024);
    spdlog::logger logger("buffer_resize", sink);
    logger.set_pattern("%v");
    logger.info("before resize");
    const auto size_before_resize = sink->get_current_size();
    sink->set_buffer_size(64 * 1024);
    REQUIRE(sink->get_current_size() == size_before_resize);
    sink->set_buffer_size(128 * 1024);
    REQUIRE(sink->get_current_size() == get_filesize("test_logs/buffer_resize.txt"));
    logger.info("after resize");
    logger.flush();
    REQUIRE(sink->get_current_size() == get_filesize("test_logs/buffer_resize.txt"));
    REQUIRE(file_contents("test_logs/buffer_resize.txt") ==
            spdlog::fmt_lib::format("header\nbefore resize{}footer\nheader\nafter resize{}",
                                   spdlog::details::os::default_eol,
                                   spdlog::details::os::default_eol));
}

//
// rotating_datetime_file_sink
//

namespace {

// Deterministic archive names, so that file existence can be asserted without
// scanning the log directory for timestamped names.
struct sequential_archive_name_calc {
    static int &counter() {
        static int value = 0;
        return value;
    }
    static spdlog::filename_t calc_filename(const spdlog::filename_t &base, const tm &) {
        return spdlog::fmt_lib::format(SPDLOG_FILENAME_T("{}-{}"), base, ++counter());
    }
};

// Always asks for the same archive name, exercising the collision handling.
struct fixed_archive_name_calc {
    static spdlog::filename_t calc_filename(const spdlog::filename_t &base, const tm &) {
        return base + SPDLOG_FILENAME_T("-fixed");
    }
};

// The same calculator type must be usable by the daily sink and by the rotating
// datetime sink (that is the "for all sinks" part of the issue).
struct shared_archive_name_calc {
    static spdlog::filename_t calc_filename(const spdlog::filename_t &base, const tm &now_tm) {
        return spdlog::fmt_lib::format(SPDLOG_FILENAME_T("{}-{:04d}{:02d}{:02d}"), base,
                                       now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);
    }
};

using rotating_datetime_sink_seq_t =
    spdlog::sinks::rotating_datetime_file_sink<spdlog::details::null_mutex,
                                               sequential_archive_name_calc>;
using rotating_datetime_sink_fixed_t =
    spdlog::sinks::rotating_datetime_file_sink<spdlog::details::null_mutex,
                                               fixed_archive_name_calc>;

}  // namespace

TEST_CASE("rotating_datetime_archive_name_format", "[rotating_datetime_logger]") {
    tm t{};
    t.tm_year = 126;  // 2026
    t.tm_mon = 9;     // october
    t.tm_mday = 23;
    t.tm_hour = 10;
    t.tm_min = 56;
    t.tm_sec = 25;

    REQUIRE(spdlog::sinks::datetime_archive_calculator::calc_filename(
                SPDLOG_FILENAME_T("logs/app.log"), t) ==
            SPDLOG_FILENAME_T("logs/app-2026-10-23_10-56-25.log"));
    REQUIRE(spdlog::sinks::datetime_archive_calculator::calc_filename(SPDLOG_FILENAME_T("logs/app"),
                                                                     t) ==
            SPDLOG_FILENAME_T("logs/app-2026-10-23_10-56-25"));
}

TEST_CASE("rotating_datetime_file_logger_rotates_on_size", "[rotating_datetime_logger]") {
    prepare_logdir();
    const std::size_t max_size = 1000;
    auto sink = std::make_shared<spdlog::sinks::rotating_datetime_file_sink_st>(
        SPDLOG_FILENAME_T(ROTATING_DATETIME_LOG), max_size, 0);
    spdlog::logger logger("rotating_datetime_sink_logger", sink);
    logger.set_pattern("%v");

    const std::string message(180, 'x');
    for (int i = 0; i < 20; ++i) {
        logger.info(message);
    }
    logger.flush();

    // at least one archive was produced, and the active file stayed near max_size
    REQUIRE(count_files("test_logs") > 1);
    REQUIRE(get_filesize(ROTATING_DATETIME_LOG) <= max_size + message.size() + 2);
}

// A size based sink must resume the file it was writing before the restart,
// otherwise max_size never applies to short lived or restarted processes.
TEST_CASE("rotating_datetime_file_logger_appends_across_restart",
          "[rotating_datetime_logger]") {
    prepare_logdir();
    const std::size_t max_size = 10 * 1024 * 1024;
    {
        auto sink = std::make_shared<spdlog::sinks::rotating_datetime_file_sink_st>(
            SPDLOG_FILENAME_T(ROTATING_DATETIME_LOG), max_size, 5);
        spdlog::logger logger("rotating_datetime_first_run", sink);
        logger.set_pattern("%v");
        logger.info("first run");
        logger.flush();
    }
    {
        auto sink = std::make_shared<spdlog::sinks::rotating_datetime_file_sink_st>(
            SPDLOG_FILENAME_T(ROTATING_DATETIME_LOG), max_size, 5);
        spdlog::logger logger("rotating_datetime_second_run", sink);
        logger.set_pattern("%v");
        logger.info("second run");
        logger.flush();
        REQUIRE(sink->get_current_size() == get_filesize(ROTATING_DATETIME_LOG));
    }

    REQUIRE(count_lines(ROTATING_DATETIME_LOG) == 2);
    REQUIRE(count_files("test_logs") == 1);
}

TEST_CASE("rotating_datetime_file_logger_prunes_old_archives", "[rotating_datetime_logger]") {
    prepare_logdir();
    sequential_archive_name_calc::counter() = 0;
    auto sink = std::make_shared<rotating_datetime_sink_seq_t>(
        SPDLOG_FILENAME_T(ROTATING_DATETIME_LOG), 1024, 3);
    spdlog::logger logger("rotating_datetime_sink_logger", sink);
    logger.set_pattern("%v");

    for (int i = 0; i < 6; ++i) {
        logger.info("Test message {}", i);
        sink->rotate_now();
    }
    logger.flush();

    REQUIRE(count_files("test_logs") == 4);  // active file + 3 archives
    REQUIRE(spdlog::details::os::path_exists(ROTATING_DATETIME_LOG "-4"));
    REQUIRE(spdlog::details::os::path_exists(ROTATING_DATETIME_LOG "-5"));
    REQUIRE(spdlog::details::os::path_exists(ROTATING_DATETIME_LOG "-6"));
    REQUIRE_FALSE(spdlog::details::os::path_exists(ROTATING_DATETIME_LOG "-1"));
}

// Two rotations within the same second ask for the same name: the second one must
// not overwrite the first archive (os::rename overwrites on POSIX).
TEST_CASE("rotating_datetime_same_second_rotations_are_distinct",
          "[rotating_datetime_logger]") {
    prepare_logdir();
    auto sink = std::make_shared<rotating_datetime_sink_fixed_t>(
        SPDLOG_FILENAME_T(ROTATING_DATETIME_LOG), 1024, 0);
    spdlog::logger logger("rotating_datetime_sink_logger", sink);
    logger.set_pattern("%v");

    logger.info("first");
    sink->rotate_now();
    logger.info("second");
    sink->rotate_now();
    logger.info("third");
    sink->rotate_now();
    logger.flush();

    REQUIRE(count_files("test_logs") == 4);
    REQUIRE(spdlog::details::os::path_exists(ROTATING_DATETIME_LOG "-fixed"));
    REQUIRE(spdlog::details::os::path_exists(ROTATING_DATETIME_LOG "-fixed-001"));
    REQUIRE(spdlog::details::os::path_exists(ROTATING_DATETIME_LOG "-fixed-002"));

    // nothing was lost to an overwritten archive
    REQUIRE(count_lines(ROTATING_DATETIME_LOG) + count_lines(ROTATING_DATETIME_LOG "-fixed") +
                count_lines(ROTATING_DATETIME_LOG "-fixed-001") +
                count_lines(ROTATING_DATETIME_LOG "-fixed-002") ==
            3);
}

TEST_CASE("rotating_datetime_file_logger_max_files_zero_keeps_all",
          "[rotating_datetime_logger]") {
    prepare_logdir();
    sequential_archive_name_calc::counter() = 0;
    auto sink = std::make_shared<rotating_datetime_sink_seq_t>(
        SPDLOG_FILENAME_T(ROTATING_DATETIME_LOG), 1024, 0);
    spdlog::logger logger("rotating_datetime_sink_logger", sink);
    logger.set_pattern("%v");

    for (int i = 0; i < 5; ++i) {
        logger.info("Test message {}", i);
        sink->rotate_now();
    }
    logger.flush();

    REQUIRE(count_files("test_logs") == 6);  // active file + 5 archives
}

TEST_CASE("rotating_datetime_file_logger_rotate_on_open", "[rotating_datetime_logger]") {
    prepare_logdir();
    {
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(
            SPDLOG_FILENAME_T(ROTATING_DATETIME_LOG));
        spdlog::logger logger("basic_before_rotate_on_open", sink);
        logger.set_pattern("%v");
        logger.info("left by the previous run");
        logger.flush();
    }
    REQUIRE(get_filesize(ROTATING_DATETIME_LOG) > 0);

    auto sink = std::make_shared<spdlog::sinks::rotating_datetime_file_sink_st>(
        SPDLOG_FILENAME_T(ROTATING_DATETIME_LOG), 1024 * 1024, 5, /*rotate_on_open=*/true);

    REQUIRE(get_filesize(ROTATING_DATETIME_LOG) == 0);
    REQUIRE(count_files("test_logs") == 2);
}

TEST_CASE("rotating_datetime_set_max_files_prunes_immediately",
          "[rotating_datetime_logger]") {
    prepare_logdir();
    sequential_archive_name_calc::counter() = 0;
    auto sink = std::make_shared<rotating_datetime_sink_seq_t>(
        SPDLOG_FILENAME_T(ROTATING_DATETIME_LOG), 1024, 0);
    spdlog::logger logger("rotating_datetime_sink_logger", sink);
    logger.set_pattern("%v");

    for (int i = 0; i < 5; ++i) {
        logger.info("Test message {}", i);
        sink->rotate_now();
    }
    logger.flush();
    REQUIRE(count_files("test_logs") == 6);

    sink->set_max_files(2);
    REQUIRE(sink->get_max_files() == 2);
    REQUIRE(count_files("test_logs") == 3);  // active file + 2 newest archives
    REQUIRE(spdlog::details::os::path_exists(ROTATING_DATETIME_LOG "-5"));
    REQUIRE_FALSE(spdlog::details::os::path_exists(ROTATING_DATETIME_LOG "-1"));
}

#ifndef SPDLOG_NO_EXCEPTIONS
TEST_CASE("rotating_datetime_file_logger_max_size_zero_throws",
          "[rotating_datetime_logger]") {
    prepare_logdir();
    REQUIRE_THROWS_AS(
        std::make_shared<spdlog::sinks::rotating_datetime_file_sink_st>(
            SPDLOG_FILENAME_T(ROTATING_DATETIME_LOG), 0, 1),
        spdlog::spdlog_ex);
}
#endif  // SPDLOG_NO_EXCEPTIONS

TEST_CASE("rotating_datetime_calculator_reused_by_daily_sink", "[rotating_datetime_logger]") {
    prepare_logdir();
    auto daily_sink =
        std::make_shared<spdlog::sinks::daily_file_sink<spdlog::details::null_mutex,
                                                        shared_archive_name_calc>>(
            SPDLOG_FILENAME_T("test_logs/shared_daily.log"), 0, 0);
    auto rotating_sink =
        std::make_shared<spdlog::sinks::rotating_datetime_file_sink<spdlog::details::null_mutex,
                                                                    shared_archive_name_calc>>(
            SPDLOG_FILENAME_T("test_logs/shared_rotating.log"), 1024);

    spdlog::logger daily_logger("shared_calc_daily", daily_sink);
    spdlog::logger rotating_logger("shared_calc_rotating", rotating_sink);
    daily_logger.set_pattern("%v");
    rotating_logger.set_pattern("%v");
    daily_logger.info("daily");
    rotating_logger.info("rotating");
    daily_logger.flush();
    rotating_logger.flush();

    REQUIRE(count_files("test_logs") == 2);
}
