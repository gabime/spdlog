#include "includes.h"
#include "test_sink.h"
#include "spdlog/fmt/bin_to_hex.h"

#if _WIN32

#include "spdlog/sinks/win_eventlog_sink.h"

static const LPCSTR TEST_LOG = "my log";
static const LPCSTR TEST_SOURCE = "my source";

static void test_single_print(std::function<void(std::string const&)> do_print, std::string const& expectedContents, WORD expectedEventType)
{
    do_print(expectedContents);

    using namespace std::chrono;
    const auto expectedTimeGenerated = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();

    struct handle_t
    {
        HANDLE handle_;

        ~handle_t()
        {
            if (handle_)
                REQUIRE(CloseEventLog(handle_));
        }
    } eventLog {OpenEventLog(nullptr, TEST_SOURCE)};

    REQUIRE(eventLog.handle_);

    DWORD readBytes {}, sizeNeeded {};
    auto ok = ReadEventLog(eventLog.handle_, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ, 0, &readBytes, 0, &readBytes, &sizeNeeded);
    REQUIRE(!ok); 
    REQUIRE(GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    std::vector<char> recordBuffer(sizeNeeded);
    PEVENTLOGRECORD record = (PEVENTLOGRECORD)recordBuffer.data();

    ok = ReadEventLog(eventLog.handle_, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ, 0, record, sizeNeeded, &readBytes, &sizeNeeded);
    REQUIRE(ok);

    REQUIRE(record->NumStrings == 1);
    REQUIRE(record->EventType == expectedEventType);
    REQUIRE(record->TimeGenerated == expectedTimeGenerated);

    std::string message_in_log(((char*) record + record->StringOffset));
    REQUIRE(message_in_log == expectedContents + "\r\n");
}

TEST_CASE("eventlog", "[eventlog]")
{
    using namespace spdlog;

    auto test_sink = std::make_shared<sinks::win_eventlog_sink_mt>(TEST_SOURCE);

    spdlog::logger test_logger("eventlog", test_sink);
    test_logger.set_level(level::trace);

    test_sink->set_pattern("%v");

    test_single_print([&test_logger] (std::string const& msg) { test_logger.debug(msg); }, "my debug message", EVENTLOG_SUCCESS);
    test_single_print([&test_logger] (std::string const& msg) { test_logger.info(msg); }, "my info message", EVENTLOG_INFORMATION_TYPE);
    test_single_print([&test_logger] (std::string const& msg) { test_logger.warn(msg); }, "my warn message", EVENTLOG_WARNING_TYPE);
    test_single_print([&test_logger] (std::string const& msg) { test_logger.error(msg); }, "my error message", EVENTLOG_ERROR_TYPE);
    test_single_print([&test_logger] (std::string const& msg) { test_logger.critical(msg); }, "my critical message", EVENTLOG_ERROR_TYPE);
}

#endif