#include "includes.h"
#include "spdlog/sinks/systemd_namespace_sink.h"
#include <chrono>

// Used as a helper struct to store journal entries when verifying the custom namespace journal
// output automatically.
struct JournalEntry {
    std::string message;
    std::string priority;
    std::string syslog_identifier;
    uint64_t realtime_usec = 0;
};

// Used as a helper class to test the custom namespace journal output automatically.
class JournalReader {
public:
    explicit JournalReader(const std::string& name_space) {
        int r = -1;
        if (name_space.empty()) {
            r = sd_journal_open(&j_, SD_JOURNAL_LOCAL_ONLY);
        } else {
            r = sd_journal_open_namespace(&j_, name_space.c_str(), SD_JOURNAL_LOCAL_ONLY);
        }
        if (r < 0) {
            throw std::runtime_error(std::string("sd_journal_open failed: ") + strerror(-r));
        }
    }

    ~JournalReader() {
        if (j_ != nullptr) {
            sd_journal_close(j_);
        }
    }

    JournalReader(const JournalReader&) = delete;
    JournalReader& operator=(const JournalReader&) = delete;

    // Position the cursor at "now" so we only see entries written after this point.
    void seek_to_now() {
        uint64_t now_usec = std::chrono::duration_cast<std::chrono::microseconds>(
                                std::chrono::system_clock::now().time_since_epoch())
                                .count();
        int r = sd_journal_seek_realtime_usec(j_, now_usec);
        if (r < 0) {
            throw std::runtime_error(std::string("seek_realtime_usec failed: ") + strerror(-r));
        }
        // After seek, you must call sd_journal_next/previous before reading.
        // We don't call next() here because there may not be an entry yet.
    }

    // Add a match filter, e.g. "SYSLOG_IDENTIFIER=myapp".
    // Multiple matches on the same field are OR'd; matches on different fields are AND'd.
    void add_match(const std::string& match) {
        int r = sd_journal_add_match(j_, match.data(), match.size());
        if (r < 0) {
            throw std::runtime_error(std::string("add_match failed: ") + strerror(-r));
        }
    }

    // Read up to max_entries new entries, waiting up to timeout for each.
    // Returns whatever was collected when either max_entries is reached or
    // a wait times out.
    std::vector<JournalEntry> wait_and_read(size_t max_entries, std::chrono::milliseconds timeout) {
        std::vector<JournalEntry> out;
        out.reserve(max_entries);

        while (out.size() < max_entries) {
            int r = sd_journal_next(j_);
            if (r < 0) {
                throw std::runtime_error(std::string("sd_journal_next failed: ") + strerror(-r));
            }
            if (r == 0) {
                // No new entry available yet; wait a moment.
                uint64_t timeout_usec = static_cast<uint64_t>(timeout.count()) * 1000;
                int w = sd_journal_wait(j_, timeout_usec);
                if (w < 0) {
                    throw std::runtime_error(std::string("sd_journal_wait failed: ") +
                                             strerror(-w));
                }
                if (w == SD_JOURNAL_NOP) {
                    // Timeout
                    break;
                }
                continue;
            }

            out.push_back(read_current_entry());
        }
        return out;
    }

private:
    JournalEntry read_current_entry() {
        JournalEntry e;

        const void* data = nullptr;
        size_t len = 0;

        if (sd_journal_get_data(j_, "MESSAGE", &data, &len) >= 0) {
            e.message = field_value(data, len);
        }
        if (sd_journal_get_data(j_, "PRIORITY", &data, &len) >= 0) {
            e.priority = field_value(data, len);
        }
        if (sd_journal_get_data(j_, "SYSLOG_IDENTIFIER", &data, &len) >= 0) {
            e.syslog_identifier = field_value(data, len);
        }
        sd_journal_get_realtime_usec(j_, &e.realtime_usec);
        return e;
    }

    // Strip "KEY=value" pair to just value.
    static std::string field_value(const void* data, size_t len) {
        const char* p = static_cast<const char*>(data);
        const char* eq = static_cast<const char*>(std::memchr(p, '=', len));
        if (!eq) return std::string(p, len);
        return std::string(eq + 1, len - (eq + 1 - p));
    }

    sd_journal* j_ = nullptr;
};

TEST_CASE("systemd_namespace to default namespace", "[all]") {
    auto logger = spdlog::systemd_namespace_logger_st("spdlog_systemd_namespace_all_test",
                                                      "spdlog-utests", "", false);
    logger->set_level(spdlog::level::trace);
    logger->trace("test spdlog trace unformatted");
    logger->debug("test spdlog debug unformatted");
    SPDLOG_LOGGER_INFO((logger), "test spdlog info unformatted");
    SPDLOG_LOGGER_WARN((logger), "test spdlog warn unformatted");
    SPDLOG_LOGGER_ERROR((logger), "test spdlog error unformatted");
    SPDLOG_LOGGER_CRITICAL((logger), "test spdlog critical unformatted");
    logger->info("Long message\nthat spans\nmultiple lines unformatted");

    auto f_logger = spdlog::systemd_namespace_logger_st("spdlog_systemd_namespace_all_test_formatted",
                                                      "spdlog-utests", "", true);
    f_logger->set_level(spdlog::level::trace);
    f_logger->trace("test spdlog trace formatted");
    f_logger->debug("test spdlog debug formatted");
    SPDLOG_LOGGER_INFO((f_logger), "test spdlog info formatted");
    SPDLOG_LOGGER_WARN((f_logger), "test spdlog warn formatted");
    SPDLOG_LOGGER_ERROR((f_logger), "test spdlog error formatted");
    SPDLOG_LOGGER_CRITICAL((f_logger), "test spdlog critical formatted");
    f_logger->info("Long message\nthat spans\nmultiple lines formatted");
}

TEST_CASE("systemd_namespace to custom namespace", "[.systemd_namespace]") {
    // This test is skipped from the default set since it requires extra preparation.
    // Running this test requires you to start journald on namespace 'custom', for example by
    // running: sudo systemctl start systemd-journald@custom.service
    //
    // This test also automatically verifies the output, so the user running the test must have
    // access to read the custom journal namespace.
    auto logger = spdlog::systemd_namespace_logger_st("spdlog_systemd_namespace_custom_test",
                                                      "spdlog-utests", "custom", false);
    JournalReader reader("custom");
    reader.add_match("SYSLOG_IDENTIFIER=spdlog-utests");
    reader.seek_to_now();

    logger->set_level(spdlog::level::trace);
    logger->trace("test spdlog trace");
    logger->debug("test spdlog debug");
    SPDLOG_LOGGER_INFO((logger), "test spdlog info");
    SPDLOG_LOGGER_WARN((logger), "test spdlog warn");
    SPDLOG_LOGGER_ERROR((logger), "test spdlog error");
    SPDLOG_LOGGER_CRITICAL((logger), "test spdlog critical");
    logger->info("Long message\nthat spans\nmultiple lines");
    logger->warn("");

    const int expected_message_count = 9;
    auto entries = reader.wait_and_read(expected_message_count, std::chrono::seconds(2));

    REQUIRE(entries.size() == expected_message_count);
    REQUIRE(entries[0].message == "test spdlog trace");
    REQUIRE(entries[0].priority == "7");
    REQUIRE(entries[1].message == "test spdlog debug");
    REQUIRE(entries[1].priority == "7");
    REQUIRE(entries[2].message == "test spdlog info");
    REQUIRE(entries[2].priority == "6");
    REQUIRE(entries[3].message == "test spdlog warn");
    REQUIRE(entries[3].priority == "4");
    REQUIRE(entries[4].message == "test spdlog error");
    REQUIRE(entries[4].priority == "3");
    REQUIRE(entries[5].message == "test spdlog critical");
    REQUIRE(entries[5].priority == "2");
    REQUIRE(entries[6].message == "Long message");
    REQUIRE(entries[6].priority == "6");
    REQUIRE(entries[7].message == "that spans");
    REQUIRE(entries[7].priority == "6");
    REQUIRE(entries[8].message == "multiple lines");
    REQUIRE(entries[8].priority == "6");
}
