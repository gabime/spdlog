#ifndef SPDLOG_NO_TZ_OFFSET

#include "includes.h"
#include <ctime>
#include <cstdlib>
#include <cstring>

// Helper to construct a simple std::tm from components
std::tm make_tm(int year, int month, int day, int hour, int minute) {
    std::tm t;
    std::memset(&t, 0, sizeof(t));
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = 0;
    t.tm_isdst = -1;
    std::mktime(&t);
    return t;
}

// Cross-platform RAII Helper to safely set/restore process timezone
class ScopedTZ {
    std::string original_tz_;
    bool has_original_ = false;

public:
    explicit ScopedTZ(const std::string& tz_name) {
        // save current TZ
#ifdef _WIN32
        char* buf = nullptr;
        size_t len = 0;
        if (_dupenv_s(&buf, &len, "TZ") == 0 && buf != nullptr) {
            original_tz_ = std::string(buf);
            has_original_ = true;
            free(buf);
        }
#else
        const char* tz = std::getenv("TZ");
        if (tz) {
            original_tz_ = tz;
            has_original_ = true;
        }
#endif

        // set new TZ
#ifdef _WIN32
        _putenv_s("TZ", tz_name.c_str());
        _tzset();
#else
        setenv("TZ", tz_name.c_str(), 1);
        tzset();
#endif
    }

    ~ScopedTZ() {
        // restore original TZ
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

using spdlog::details::os::utc_minutes_offset;

TEST_CASE("UTC Offset - Western Hemisphere (USA - Standard Time)", "[timezone][west]") {
    // EST5EDT: Eastern Standard Time (UTC-5)
    ScopedTZ tz("EST5EDT");

    // Jan 15th (Winter)
    auto tm = make_tm(2023, 1, 15, 12, 0);
    REQUIRE(utc_minutes_offset(tm) == -300);
}

TEST_CASE("UTC Offset - Eastern Hemisphere (Europe/Israel - Standard Time)", "[timezone][east]") {
    // IST-2IDT: Israel Standard Time (UTC+2)
    ScopedTZ tz("IST-2IDT");

    // Jan 15th (Winter)
    auto tm = make_tm(2023, 1, 15, 12, 0);
    REQUIRE(utc_minutes_offset(tm) == 120);
}

TEST_CASE("UTC Offset - Zero Offset (UTC/GMT)", "[timezone][utc]") {
    ScopedTZ tz("GMT0");

    // Check Winter
    auto tm_winter = make_tm(2023, 1, 15, 12, 0);
    REQUIRE(utc_minutes_offset(tm_winter) == 0);

    // Check Summer (GMT never shifts, so this should also be 0)
    auto tm_summer = make_tm(2023, 7, 15, 12, 0);
    REQUIRE(utc_minutes_offset(tm_summer) == 0);
}

TEST_CASE("UTC Offset - Non-Integer Hour Offsets (India)", "[timezone][partial]") {
    // IST-5:30: India Standard Time (UTC+5:30)
    ScopedTZ tz("IST-5:30");

    auto tm = make_tm(2023, 1, 15, 12, 0);
    REQUIRE(utc_minutes_offset(tm) == 330);
}

TEST_CASE("UTC Offset - Edge Case: Negative Offset Crossing Midnight", "[timezone][edge]") {
    ScopedTZ tz("EST5EDT");
    // Late night Dec 31st, 2023
    auto tm = make_tm(2023, 12, 31, 23, 59);
    REQUIRE(utc_minutes_offset(tm) == -300);
}

TEST_CASE("UTC Offset - Edge Case: Leap Year", "[timezone][edge]") {
    ScopedTZ tz("EST5EDT");
    // Feb 29, 2024 (Leap Day) - Winter
    auto tm = make_tm(2024, 2, 29, 12, 0);
    REQUIRE(utc_minutes_offset(tm) == -300);
}

TEST_CASE("UTC Offset - Edge Case: Invalid Date (Pre-Epoch)", "[timezone][edge]") {
#ifdef _WIN32
    // Windows mktime returns -1 for dates before 1970.
    // We expect the function to safely return 0 (fallback).
    auto tm = make_tm(1960, 1, 1, 12, 0);
    REQUIRE(utc_minutes_offset(tm) == 0);
#else
    // Unix mktime handles pre-1970 dates correctly.
    // We expect the actual historical offset (EST was UTC-5 in 1960).
    ScopedTZ tz("EST5EDT");
    auto tm = make_tm(1960, 1, 1, 12, 0);
    REQUIRE(utc_minutes_offset(tm) == -300);
#endif
}

#endif  // !SPDLOG_NO_TZ_OFFSET