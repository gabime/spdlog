#ifndef SPDLOG_NO_TZ_OFFSET

#include "includes.h"

#include <ctime>
#include <cstdlib>

// Helper to construct a simple std::tm from components
std::tm make_tm(int year, int month, int day, int hour, int minute) {
    std::tm t{};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = 0;
    t.tm_isdst = -1;  // -1 = Let OS decide the if there was day light saving at that date
    std::mktime(&t);

    return t;
}

// RAII Helper to safely set/restore process timezone
class ScopedTZ {
    std::string original_tz_;
    bool has_original_ = false;

public:
    explicit ScopedTZ(const std::string& tz_name) {
        // Save current TZ
#ifdef _WIN32
        char* buf = nullptr;
        size_t len = 0;
        if (_dupenv_s(&buf, &len, "TZ") == 0 && buf != nullptr) {
            original_tz_ = std::string(buf);
            has_original_ = true;
            free(buf);
        }
        // Set new TZ
        _putenv_s("TZ", tz_name.c_str());
        _tzset();
#else
        const char* tz = std::getenv("TZ");
        if (tz) {
            original_tz_ = tz;
            has_original_ = true;
        }
        setenv("TZ", tz_name.c_str(), 1);
        tzset();
#endif
    }

    ~ScopedTZ() {
        // Restore original TZ
#ifdef _WIN32
        if (has_original_) {
            _putenv_s("TZ", original_tz_.c_str());
        } else {
            _putenv_s("TZ", "");
        }
        _tzset();
#else
        if (has_original_) {
            setenv("TZ", original_tz_.c_str(), 1);
        } else {
            unsetenv("TZ");
        }
        tzset();
#endif
    }
};

//
// Test the `utc_minutes_offset(const std::tm &tm)` function under various timezones and dates.
//
using spdlog::details::os::utc_minutes_offset;

TEST_CASE("UTC Offset - Western Hemisphere (USA)", "[timezone][west]") {
    // EST5EDT: Eastern Standard Time (UTC-5), Eastern Daylight Time (UTC-4)
    ScopedTZ tz("EST5EDT");

    SECTION("Standard Time (Winter)") {
        // Jan 15th, 2023 @ 12:00 PM
        auto tm = make_tm(2023, 1, 15, 12, 0);

        // Expected: UTC-5 (-300 minutes)
        REQUIRE(utc_minutes_offset(tm) == -300);
    }

    SECTION("Daylight Saving Time (Summer)") {
        // Jul 15th, 2023 @ 12:00 PM
        auto tm = make_tm(2023, 7, 15, 12, 0);

        // Expected: UTC-4 (-240 minutes)
        REQUIRE(utc_minutes_offset(tm) == -240);
    }
}

TEST_CASE("UTC Offset - Eastern Hemisphere (Europe/Israel)", "[timezone][east]") {
    // IST-2IDT: Israel Standard Time (UTC+2), Israel Daylight Time (UTC+3)
    // Note: POSIX TZ strings use POSITIVE numbers for West of Greenwich.
    // So "UTC+2" is written as "IST-2".
    ScopedTZ tz("IST-2IDT");

    SECTION("Standard Time (Winter)") {
        // Jan 15th, 2023
        auto tm = make_tm(2023, 1, 15, 12, 0);

        // Expected: UTC+2 (+120 minutes)
        REQUIRE(utc_minutes_offset(tm) == 120);
    }

    SECTION("Daylight Saving Time (Summer)") {
        // Aug 15th, 2023
        auto tm = make_tm(2023, 8, 15, 12, 0);

        // Expected: UTC+3 (+180 minutes)
        REQUIRE(utc_minutes_offset(tm) == 180);
    }
}

TEST_CASE("UTC Offset - Zero Offset (UTC/GMT)", "[timezone][utc]") {
    // GMT0: Always UTC, no DST
    ScopedTZ tz("GMT0");

    SECTION("Winter") {
        auto tm = make_tm(2023, 1, 15, 12, 0);
        REQUIRE(utc_minutes_offset(tm) == 0);
    }

    SECTION("Summer") {
        auto tm = make_tm(2023, 7, 15, 12, 0);
        REQUIRE(utc_minutes_offset(tm) == 0);
    }
}

TEST_CASE("UTC Offset - Non-Integer Hour Offsets (India)", "[timezone][partial]") {
    // IST-5:30: India Standard Time (UTC+5:30)
    // No DST usually.
    ScopedTZ tz("IST-5:30");

    SECTION("Standard Year Round") {
        // Jan 15th
        auto tm = make_tm(2023, 1, 15, 12, 0);

        // Expected: 5.5 hours * 60 = 330 minutes
        REQUIRE(utc_minutes_offset(tm) == 330);
    }
}

TEST_CASE("UTC Offset - Edge Case: Negative Offset Crossing Midnight", "[timezone][edge]") {
    // EST5EDT (New York)
    ScopedTZ tz("EST5EDT");

    // Late night Dec 31st, 2023
    auto tm = make_tm(2023, 12, 31, 23, 59);

    // Should still be Standard Time (Winter) -> UTC-5
    REQUIRE(utc_minutes_offset(tm) == -300);
}

TEST_CASE("UTC Offset - Edge Case: Leap Year", "[timezone][edge]") {
    // EST5EDT
    ScopedTZ tz("EST5EDT");

    // Feb 29, 2024 (Leap Day) - Winter
    auto tm = make_tm(2024, 2, 29, 12, 0);

    // Should be valid and return Standard offset
    REQUIRE(utc_minutes_offset(tm) == -300);
}

#endif  // !SPDLOG_NO_TZ_OFFSET