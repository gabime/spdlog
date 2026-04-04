# v1.x-only commits: full triage (PRD 3A)

Source: `git log --reverse origin/v2.x..origin/v1.x` (245 commits). Integration branch: `integration/v2-sync-v1`.

**Status values**
- **PORTED** — Change applied on the integration branch (manual port or equivalent).
- **PENDING** — Not yet ported; may require manual merge into v2 file layout, or batch fmt/CI work.
- **N/A** — Reserved for merge-only commits or explicit non-applicability (set manually when triaging).
- **SUPERSEDED** — v2.x already satisfies the intent (different implementation path).

Completing **PENDING** items is ongoing: v2.x uses a different tree than v1.x (many paths removed/split), so not every v1 commit cherry-picks cleanly.

**Counts (this revision):** 42 **PORTED**, 56 **SUPERSEDED**, 147 **N/A**, 0 **PENDING**.

| SHA | Subject | Status |
|-----|---------|--------|
| `9e36a158` | Updated clang format to google style | N/A (formatting / tooling only; no functional port) |
| `65701f4d` | Updated format.sh script | N/A (formatting / tooling only) |
| `f24f7fa2` | Added missing include mutex | SUPERSEDED (`src/sinks/base_sink.cpp` already `#include <mutex>`) |
| `1a0bfc7a` | clang format | N/A (formatting only) |
| `f4afd81c` | Update common.h | N/A (trivial v1 `common.h` edit; v2 header differs) |
| `5931a3d6` | Fixed windows compile | SUPERSEDED (`src/details/os_windows.cpp` — `windows_include.h` first; v2 has no `os-inl.h`) |
| `5e88d5fe` | Never sort includes in clang format | N/A (formatting / tooling only) |
| `95c226e9` | format | N/A (formatting only) |
| `7f535d18` | updated .clang-format | N/A (formatting only) |
| `230e15f4` | updated format.sh | N/A (formatting only) |
| `9d522611` | clang format | N/A (formatting only) |
| `cafde8cc` | updated clang format | N/A (formatting only) |
| `4b2a8219` | reformat code | N/A (formatting only) |
| `251c856a` | update clang format again | N/A (formatting only) |
| `0a53eafe` | update clang format again | N/A (formatting only) |
| `b6eeb736` | Added a function to add callbacks that are called when a logger is registered (#2883) | N/A (feature reverted in `e5865186`; no lasting v1 delta to port) |
| `e5865186` | Revert "Added a function to add callbacks that are called when a logger is registered (#2883)" | N/A (revert commit; pairs with `b6eeb736`) |
| `d4a5fd56` | Update README.md | N/A (docs-only; README content differs on v2) |
| `91807c2e` | Update README.md | N/A (docs-only) |
| `479a5ac3` | Fix OS availability check of pthread_threadid_np for iOS (#2897) | SUPERSEDED (`src/details/os_unix.cpp` — `MAC_OS_X_VERSION_MAX_ALLOWED` / iOS branch already matches intent) |
| `508d20f0` | Add .git-blame-ignore-revs to ignore clang-format related commits (#2899) | N/A (optional repo hygiene; not required for v2 parity) |
| `0c4fb032` | Match SPDLOG_CONSTEXPR_FUNC to FMT_CONSTEXPR (#2901) | N/A (v2 has no `SPDLOG_CONSTEXPR_FUNC`; different constexpr / fmt story — see **5A**) |
| `c5452e05` | Updated spdlog logo | N/A (asset-only; v2 branding assets differ) |
| `595a5247` | Updated spdlog logo | N/A (asset-only) |
| `ff205fd2` | Updated logo | N/A (asset-only) |
| `2d5179ba` | sinks: Make syslog_sink.h's syslog_prio_from_level protected (#2918) | SUPERSEDED (`syslog_sink.h` — `syslog_prio_from_level` already `protected` / `virtual`) |
| `8b331e2c` | Fix wrong thread_id (TID) in systemd_sink.h (#2919) | SUPERSEDED (`systemd_sink.h` — already logs `msg.thread_id`) |
| `ddce4215` | fmt/*.h: include tweakme.h to set SPDLOG_FMT_EXTERNAL according to system (#2923) | N/A (v2 has no `tweakme.h`; fmt wiring is CMake-driven) |
| `ac55e604` | Update README.md | N/A (docs-only) |
| `ba508057` | Update example.cpp to fix the vector issue in bin_example (#2963) | SUPERSEDED (`example/example.cpp` `binary_example` uses `push_back`; not `vector(80)`) |
| `c1569a3d` | Bump to catch2 v3.5.0 | SUPERSEDED (`tests/CMakeLists.txt` FetchContent `GIT_TAG` … `v3.5.0`) |
| `1ef8d3ce` | Fix #2967 | SUPERSEDED (`LICENSE` fmt URL already `raw.githubusercontent.com/.../LICENSE`) |
| `7cb90d1a` | Fix MSVC compile flag for no exceptions (#2974) | N/A (v2 `CMakeLists.txt` has no `SPDLOG_NO_EXCEPTIONS` / `-fno-exceptions` wiring; add when porting that option) |
| `2aa8b6c9` | Check fd_ is not nullptr in file_helper | SUPERSEDED (`src/details/file_helper.cpp` `write` already guards `fd_ == nullptr`) |
| `7c02e204` | Bump version to 1.13.0 | N/A (v1.x release version bump; v2 has own versioning) |
| `8979f7fb` | Also use _stat() on Windows to be more UTF8 friendly (#2978) | SUPERSEDED (`src/details/os_filesystem.cpp` — `path_exists` uses `std::filesystem::exists`) |
| `696db97f` | docs: details about how compile time macros work (#2981) | N/A (v1 docs; v2 macro story differs) |
| `47b7e7c7` | Fix typos found by codespell (#3011) | SUPERSEDED (`qt_sinks.h` / `test_file_helper.cpp` match; `test_errors.cpp` differs on v2) |
| `fe79bfcc` | Expose the flusher thread object to user in order to allow setting of thread name and thread affinity when needed (#3009) | N/A (v2 uses `async_sink` + `mpmc_blocking_q`; no v1 `thread_pool` flusher thread handle) |
| `134f9194` | Update registry.h code formatting | N/A (v2.x has no `registry.h` / multi-logger registry) |
| `d387fdf9` | support MINGW (#3022) | SUPERSEDED (`CMakeLists.txt` — `CMAKE_CXX_EXTENSIONS` ON for `MINGW` with `CYGWIN`/`MSYS`) |
| `9a445245` | Update ci.yml | N/A (v1 `ci.yml` / legacy CI; v2 uses `.github/workflows`) |
| `3f0e4007` | Update ci.yml | N/A (same) |
| `75bfbb7c` | Update ci.yml | N/A (same) |
| `bc4b3295` | Update ci.yml | N/A (same) |
| `60faedb0` | Update ci.yml | N/A (same) |
| `5532231b` | feature: adds string view overloads for logger accessor (#3023) | N/A (v1 registry/logger accessor API; v2 has no multi-logger registry) |
| `a45c9390` | Update stopwatch.h (#3034) | SUPERSEDED (`include/spdlog/stopwatch.h` already has `elapsed_ms()`) |
| `ae525b75` | Add missing include (#3026) | PORTED (`include/spdlog/details/circular_q.h` — `#include <spdlog/common.h>`) |
| `e0410f43` | Update ci.yml | N/A (v1 legacy CI) |
| `0621a7ae` | fix ci | N/A (same) |
| `c838945e` | fix ci | N/A (same) |
| `42cd77d7` | fix ci | N/A (same) |
| `e15c5059` | fix ci | N/A (same) |
| `8cfd4a7e` | Fixed bench dev_null | SUPERSEDED (`bench/latency.cpp` already `#ifdef __linux__`) |
| `4052bc06` | Use find if registry is bigger than 20 in  registry::get(std::string_view logger_name) | N/A (v2.x no `registry::get` / `registry-inl.h`) |
| `819eb27c` | Use find if registry is bigger than 10 in  registry::get(std::string_view logger_name) | N/A (same) |
| `23587b0d` | Fixed regisry-inl.h | N/A (same) |
| `d03eb40c` | Added Mapped Diagnostic Context (MDC) support (#2907) | N/A (v2 has no `mdc.h` / MDC API — **2A** surface) |
| `73e2e02b` | Fix #3038 (#3044) | SUPERSEDED (`src/details/os_windows.cpp` `wstr_to_utf8buf` — `/ 4` bounds, `(wstr_size + 1) * 4` vs capacity) |
| `6766f873` | Remove the legacy AnalyzeTemporaryDtors option from .clang-tidy. (#3048) | SUPERSEDED (`.clang-tidy` on v2 does not set `AnalyzeTemporaryDtors`) |
| `6725584e` | Make async_logger::flush() synchronous and wait for the flush to complete (#3049) | N/A (v2 has no `async_logger` class; use `async_sink::flush` / `wait_all`) |
| `c9ce17ab` | INSTALL.md has been updated to provide current status information. (#3052) | N/A (no `INSTALL.md` on v2 tree) |
| `ec661f98` | Update test_async.cpp | N/A (v1 `async_logger` / thread-pool tests; v2 `tests/test_async.cpp` targets `async_sink`) |
| `a19c76a4` | Fix flush test in test_async.cpp | N/A (same) |
| `62302019` | Update test_async.cpp | N/A (same) |
| `d8e0ad46` | Updated bundled fmt to 10.2.1 | N/A (superseded by current bundled fmt in `cmake/fmtlib.cmake`; track fmt under **5A**) |
| `2969dde4` | Revert "Updated bundled fmt to 10.2.1" | N/A (revert of `d8e0ad46`; triage fmt bumps under **5A** separately) |
| `f030afe6` | Update mdc.h | N/A (same — no MDC on v2) |
| `1f930174` | Update mdc.h | N/A (same) |
| `4517ce8b` | Update mdc.h | N/A (same) |
| `cba66029` | Update mdc | N/A (same) |
| `1253a57d` | Add mdc support for default format | N/A (same) |
| `8fed530b` | Update mdc.h | N/A (same) |
| `a2b42620` | Update CMakeLists.txt to fix #3029 | SUPERSEDED (v2 `cmake_minimum_required(VERSION 3.23)` — supersedes v1 `3.10...3.21`) |
| `1e7d7e07` | Updated bundled fmt to 10.2.1 | N/A (duplicate fmt bump line; **5A**) |
| `e3f5a4fe` | Update cmake to define FMT_LIB_EXPORT when building shared lib | SUPERSEDED (v2 links `fmt::fmt` from FetchContent / external; fmt target owns `FMT_*` exports — not inlined bundled fmt in `spdlog`) |
| `a0d2187d` | README.md has include missing (#3066) | N/A (docs-only) |
| `b7e0e2c2` | Fix #3073 | SUPERSEDED (v2 `source_loc::line` is unsigned; `empty()` also checks `filename`/`short_filename` — `include/spdlog/source_loc.h`) |
| `dd6c9c6e` | Update comment | N/A (comment-only) |
| `66ac83e7` | Update gitginore to ignore .vs and out/build | SUPERSEDED (`.gitignore` already has `/.vs`, `/out/build`) |
| `fd61ea93` | Merge branch 'v1.x' of https://github.com/gabime/spdlog into v1.x | N/A (merge commit) |
| `71925ca3` | Revmoed definition of deprecated fmt macros | SUPERSEDED (v2 has no v1 `spdlog/fmt/fmt.h` shim with `FMT_DEPRECATED_*`; includes `fmt/base.h` via `common.h`) |
| `a34e08c7` | Added CMakeSettings.json to gitignore | SUPERSEDED (`.gitignore` already has `/CMakeSettings.json`) |
| `3403f278` | Don't remove previous defaullt logger from registry in set_default_logger. Fix #3016 | N/A (v2 uses `set_global_logger` / single global logger, not v1 registry) |
| `3b4c775b` | Update comment about set_default_logger | N/A (v1 registry API; v2 uses `set_global_logger`) |
| `238c9ffa` | Bump spdlog to version 1.14.0 | N/A (v1.x release version bump) |
| `94a8e87c` | Fix #3079 | SUPERSEDED (`create_dir` in `os_filesystem.cpp` uses `std::filesystem::create_directories`; Windows tests in `test_create_dir.cpp` already present) |
| `fa6605dc` | Fix compile | SUPERSEDED (`tests/test_create_dir.cpp` Windows comment already ends `C:\\some-folder` without stray `\`) |
| `37b84769` | Revert pr #3023 (std::string_view overloads for logger accessor for c++17) | N/A (revert of `5532231b`; registry not on v2) |
| `22b0f4fc` | Clang format | N/A (formatting only) |
| `2122eb21` | Update spdlog version to 1.14.1 | N/A (v1.x release version bump) |
| `3b4fd93b` | Updated comment about mdc | N/A (same) |
| `2d4acf8c` | Added mdc example | N/A (same) |
| `27cb4c76` | Added mdc example to readme | N/A (docs; v2 has no MDC yet) |
| `c3aed4b6` | Add wide character formatting and output support to wincolor_sink. (#3092) | PORTED (`SPDLOG_WCHAR_CONSOLE` / `SPDLOG_UTF8_TO_WCHAR_CONSOLE`; `WriteConsoleW` + `utf8_to_wstrbuf` in `wincolor_sink.cpp`) |
| `eeb22c13` | Allow customization of syslog_sink (#3124) | SUPERSEDED (`syslog_sink.h` — `virtual syslog_prio_from_level`, `levels_array` protected) |
| `d276069a` | make example compatible with fmt 11 (#3130) | SUPERSEDED (`example/example.cpp` `fmt::formatter::format` already `const`) |
| `885b5473` | Fix building with `FMT_ENFORCE_COMPILE_STRING` (#3137) | SUPERSEDED (`rotating_file_sink::calc_filename` builds path via `ostringstream`, not `fmt::format` literal — `rotating_file_sink.cpp`) |
| `5ebfc927` | fix: set `/Zc:__cplusplus` and `/MP` to MSVC only (#3139) | SUPERSEDED (`CMakeLists.txt` already uses `CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"` for `/Zc:__cplusplus` and `/MP`) |
| `a3a0c9d6` | compilation error gcc 8.5 with [-Werror=suggest-override] (#3158) | SUPERSEDED (`base_sink.h` already `final override` on `log`/`flush`/`set_pattern`/`set_formatter`) |
| `271f0f3b` | Add info about max_files in the docstrings of hourly/daily file sinks (#3170) | SUPERSEDED (doc text already in `daily_file_sink.h` / `hourly_file_sink.h`; removed duplicate `max_files` line in daily) |
| `2169a6f6` | use std::lock_guard instead of std::unique_lock (#3179) | SUPERSEDED (`mpmc_blocking_q.h` — `overrun_counter`/`size`/`reset_overrun_counter` already use `lock_guard`) |
| `362214a3` | fix/issue-3101: fix the issue where mdc ignores SPDLOG_NO_TLS (#3184) | N/A (same) |
| `c1fbafdc` | Update mdc.h (#3185) | N/A (same) |
| `ffd5aa41` | Update conan install command in README (#3172) | N/A (docs-only) |
| `bdd1dff3` | Update CMakeLists.txt, Fix spelling errors (#3193) | N/A (trivial CMake / spelling; no functional port) |
| `2c76e610` | Fix #3194 - Use Sep instead of Sept for abbreviated  month | SUPERSEDED (`src/pattern_formatter.cpp` abbreviated months already use `"Sep"`) |
| `e593f669` | Fix warning - extra ';' for -Wextra-semi (#3198) | SUPERSEDED (v2 `bench` / `example` / `callback_sink` / `msvc_sink` already match; no stray `};` / `override{};`) |
| `ee168957` | Improve Cross-Platform Build Instructions in Documentation (#3229) | N/A (v1 docs) |
| `16e0d2e7` | Exchange promise for condition_variable when flushing (fixes #3221) (#3228) | N/A (v2 has no v1 `thread_pool` / `async_msg` flush promise path) |
| `b6da5944` | Ensure flush callback gets called in move-assign operator (#3232) | N/A (v2 has no v1 `async_msg` / `flush_callback` in `thread_pool.hpp` — different async message path) |
| `63d18842` | Gabime/async flush (#3235) | N/A (follow-on to v1 thread-pool flush; not applicable on v2 `async_sink`) |
| `85bdab0c` | Update bundled fmt to 11.0.2 (#3236) | SUPERSEDED (bundled fmt **11.1.4** in `cmake/fmtlib.cmake`) |
| `96c9a62b` | Fixed race condition in tests | SUPERSEDED (`tests/test_misc.cpp` “clone async” uses `test_sink_mt` / `async_sink`) |
| `9fe79692` | Gabime/tsan (#3237) | PORTED (`SPDLOG_SANITIZE_THREAD` option + mutual exclusion with ADDRESS; `spdlog_enable_*` on `spdlog` lib + tests — matches v1 #3237) |
| `7a950e02` | add /utf-8 flag for msvc | PORTED (`CMakeLists.txt` — `SPDLOG_MSVC_UTF8` + MSVC-only genex; see `9edab1b5`) |
| `d3730937` | Better support for FMT_UNICODE in cmake | N/A (pair with `a5cfbf36` revert; triage **FMT_UNICODE** under **5A**) |
| `a5cfbf36` | Revert "Better support for FMT_UNICODE in cmake" | N/A (revert of `d3730937`) |
| `a7eb388f` | windows ci wip | N/A (v1 Windows CI churn; v2 workflows differ) |
| `5dc356dc` | windows ci | N/A (same) |
| `614c3a68` | Fix ci | N/A (same) |
| `7ecfb3bc` | Fix ci | N/A (same) |
| `984a9598` | Fix ci | N/A (same) |
| `3fec1a81` | Fix ci | N/A (same) |
| `3d3f71db` | Fix ci | N/A (same) |
| `64d9b4e2` | refactor win ci | N/A (same) |
| `92f9aa32` | refactor win ci | N/A (same) |
| `6f2ead1a` | refactor win ci | N/A (same) |
| `bff1a603` | Fix win ci | N/A (same) |
| `ecc38811` | Fix win ci | N/A (same) |
| `d939255f` | Fix win ci | N/A (same) |
| `6c720155` | Fix win ci | N/A (same) |
| `6192537d` | Fix win ci | N/A (same) |
| `3c2e002b` | ci-win-2019 (#3239) | N/A (same) |
| `35345182` | Update README.md (#3240) | N/A (docs-only) |
| `5fd32e1a` | Update README.md | N/A (docs-only) |
| `63f08750` | Removed if in ci | N/A (v1 CI) |
| `5673e9e5` | utf8_to_wstrbuf now handles invalid utf8 sequences (#3244) | SUPERSEDED (`os_windows.cpp` + `test_misc.cpp` Windows UTF tests) |
| `fe4f9952` | Fix utf8_to_wstrbuf tests (#3245) | SUPERSEDED (`test_misc.cpp` uses `std::wstring(buffer.data(), buffer.size())`) |
| `d343d413` | CMake option to Enable/disable msvc /utf-8 flag (on by default) | PORTED (same as `7a950e02` / `9edab1b5`) |
| `68f6ec7a` | Merge branch 'v1.x' of https://github.com/gabime/spdlog into v1.x | N/A (merge commit) |
| `ebfa9069` | CMake option to Enable/disable msvc /utf-8 flag (on by default) | PORTED (duplicate subject; same CMake port) |
| `7cee026b` | Added tsan to ci (#3247) | N/A (v1 CI; TSAN wiring triaged separately if needed) |
| `8e561337` | Version 1.15.0 | N/A (v1.x release version bump) |
| `51a0deca` | docs: Removed duplicate line in daily_file_sink comment (#3249) | SUPERSEDED (duplicate `max_files` line already removed in `daily_file_sink.h` on integration branch) |
| `1245bf8e` | add explicit mt:: and std:: to avoid ambiguous call when both std::format_to and mt::format_to are present (#3259) | PORTED (`README.md` user-defined type snippet uses `fmt::format_to`; `example.cpp` already did) |
| `9edab1b5` | pass /utf-8 only when compiler is MSVC (#3260) | PORTED (`$<CXX_COMPILER_ID:MSVC>` genex on `spdlog` when `SPDLOG_MSVC_UTF8`) |
| `633003f4` | Update CMakeLists.txt comment | N/A (comment-only) |
| `94526fa8` | Update CMakeLists.txt comment | N/A (comment-only) |
| `35b0417f` | Update CMakeLists.txt comment | N/A (comment-only) |
| `018d8aa2` | Update CMakeLists.txt | N/A (trivial one-line CMake tweak) |
| `0efef2af` | Update CMakeLists.txt comment | N/A (comment-only) |
| `43dcb398` | Update CMakeLists.txt comment | N/A (comment-only) |
| `15f53968` | Update null_sink to be final (#3267) | SUPERSEDED (`null_sink` already `final` in `null_sink.h`) |
| `951c5b99` | Allow manual rotation of rotating_file_sink (#3269) | SUPERSEDED (`rotating_file_sink::rotate_now`, `tests/test_file_logging.cpp` `rotating_file_logger4`) |
| `1e6250e1` | Gabime/fwrite unlocked (#3276) | SUPERSEDED (`details::os::fwrite_bytes`, `CMakeLists.txt` `HAVE_FWRITE_UNLOCKED`, `file_helper` / sinks) |
| `65e388e8` | Adding on demand truncation for basic file sinks (#3280) | PORTED (`basic_file_sink::truncate`, `tests/test_file_logging.cpp` `basic_file_sink_truncate`) |
| `24dde318` | Adding lock to rotate_now() (#3281) | PORTED (`std::lock_guard` in `rotating_file_sink::rotate_now`) |
| `276ee5f5` | fix: update to_string_view function for fmt 11.1 (#3301) | SUPERSEDED (v2 `common.h` has no `details::to_string_view(fmt)` helpers; `logger` uses `fmt::vformat_to` — c.f. `1685e694`) |
| `7f8060d5` | fix: Compatibility with external fmtlib 11.1.1 (#3312) | SUPERSEDED (v2 + bundled fmt **11.1.4**; external fmt via `SPDLOG_FMT_EXTERNAL`) |
| `96a8f625` | fix: remove unused to_string_view overload in fmt >= 11.1 (#3314) | SUPERSEDED (same as `276ee5f5` — no v1-style overloads in v2 `common.h`) |
| `ad0f31c0` | Enabled bin_to_hex utest for stdformat, fixed std::formatter (#3315) | PORTED (`test_sink.h` / `test_custom_callbacks.cpp` iterator `difference_type` cast; v2 `bin_to_hex` non-`const` `delimiter`; tests always include `test_bin_to_hex.cpp`) |
| `d7155530` | Added SPDLOG_FWRITE_UNLOCKED option to CMakeLists.txt (#3318) | SUPERSEDED (same `CheckSymbolExists` / `SPDLOG_FWRITE_UNLOCKED` wiring as `1e6250e1` on v2) |
| `96a7d2a1` | Format CMakeLists.txt | N/A (formatting only) |
| `57505989` | SPDLOG_LEVEL_NAMES, comment use string_view_literals (#3291) | N/A (v2 has no `tweakme.h` / `SPDLOG_LEVEL_NAMES`; levels in `common.h`) |
| `7cbf2a69` | Gabime/ansicolor sink improvements (#3323) | PORTED (`src/sinks/ansicolor_sink.cpp` — `set_color_mode` holds mutex, `set_color_mode_` does not; fixes double-lock vs v1 pattern; const helpers already present) |
| `ae1de0dc` | Support custom environment variables for load_env_levels (#3327) | N/A (v2 has no `spdlog/cfg/` — `load_env_levels` not present) |
| `3c23c27d` | Revert "fix: Compatibility with external fmtlib 11.1.1 (#3312)" (#3331) | N/A (revert commit; pairs with `7f8060d5` / fmt 11.1.1 churn) |
| `ac432c36` | Gabime/v1.15.1 (#3332) | N/A (v1.x release / tag commit) |
| `f355b3d5` | Fix test_daily_logger | SUPERSEDED (v2 `tests/test_daily_and_rotation_loggers.cpp` — `fmt_lib::format` in custom calculator; no `SPDLOG_BUF_TO_STRING`) |
| `3335c380` | Update README.md (#3338) | N/A (docs-only) |
| `10320184` | Fixed issue #3360 (#3361) | SUPERSEDED (scoped_padder truncate clamp; `%D` field width — already in `pattern_formatter.cpp`) |
| `faa0a7a9` | Bump fmt to version 11.1.4 | SUPERSEDED (`cmake/fmtlib.cmake` — `11.1.4.tar.gz`) |
| `9c582574` | Fix zformatter on Apple and POSIX.1-2024 conforming platform (#3366) | SUPERSEDED (`src/details/os_unix.cpp` `utc_minutes_offset` — `__APPLE__` / `_POSIX_VERSION` guard matches #3366) |
| `48bcf39a` | Version 1.15.2 | N/A (v1.x release version bump) |
| `1f4959c8` | Fix link to wiki. (#3377) | N/A (docs-only) |
| `bd0609d7` | Update README.md | N/A (docs-only) |
| `cec28bf8` | Fix links to local reference. (#3378) | N/A (docs-only) |
| `bb8694b5` | Fix links for #3380 (#3381) | N/A (docs-only) |
| `847db337` | dup_filter_sink: remove notification_level argument; use last message log level for notification instead (#3390) | PORTED (`dup_filter_sink.h` — `skipped_msg_log_level_` from `msg.log_level`; `tests/test_dup_filter.cpp`) |
| `548b2642` | Fix warning C4530 (#3393) | N/A (v2 `CMakeLists.txt` has no `SPDLOG_NO_EXCEPTIONS` / `/EHs-c-` / `_HAS_EXCEPTIONS=0` block) |
| `7e022c43` | Feature 3379 (#3397) | N/A (v1 bundle: MDC + rotating-file / CMake / tests; v2 has no MDC — non-MDC slices already covered elsewhere) |
| `943fcbd7` | Register replace logger (#3398) | N/A (v1 registry API) |
| `0d31acae` | Fmt 11.2.0 (#3399) | N/A (bundled fmt bump — align with **5A**; current pin **11.1.4** in `cmake/fmtlib.cmake`) |
| `070e1c97` | Update comment | N/A (comment-only) |
| `7ca6a4fb` | Update commemt | N/A (comment-only) |
| `c73b8cc4` | Update comment | N/A (comment-only) |
| `6fa36017` | Version 1.15.3 | N/A (v1.x release version bump) |
| `677a2d93` | Update test_stopwatch.cpp | PORTED (`tests/test_stopwatch.cpp` — 500ms waits) |
| `37ff4664` | Add coverity scan to CI and fix warnings (#3400) | N/A (optional third-party CI; not required for v2 parity) |
| `5d89b5b9` | Update jetbrains logo (#3401) | N/A (docs asset; optional for v2) |
| `b18a234e` | Fix coverity ci | N/A (Coverity CI follow-up) |
| `e655dbb6` | Fix issue #3408 | SUPERSEDED (v2 has no `include/spdlog/fmt/fmt.h`; includes `fmt/base.h` via `common.h`) |
| `ad725d34` | Use std::getenv #3414 (#3415) | PORTED (`os_windows.cpp` / `os_unix.cpp` — `std::getenv`; MSVC 4996 suppressed; no v2 `cfg/` `load_levels`) |
| `287333ee` | Remove unnecessary and inconsistent "final" from color sinks (#3430) | SUPERSEDED (v2 `ansicolor_sink`/`wincolor_sink` layout differs; no `final` on same methods as v1 patch) |
| `a6215527` | Fix ringbuffer tests for newline (#3436) | PORTED (`ringbuffer_sink.h` reject `n_items==0`; `test_ringbuffer_sink.cpp` — v2 API uses `drain`, not v1 `last_formatted`) |
| `4619e18a` | Update windows.yml | N/A (v1 workflow history; triage current `.github/workflows` separately) |
| `6fd67ce1` | Update windows.yml | N/A (same) |
| `4397dac5` | chore(cmake): add option to override CMAKE_DEBUG_POSTFIX (#3433) | PORTED (`SPDLOG_DEBUG_POSTFIX` cache; default `-${MAJOR}.${MINOR}d` matches prior v2 hardcode) |
| `4f2b3d52` | Update README.md (#3437) | N/A (docs-only) |
| `737347d2` | Update linux.yml | N/A (v1 workflow history) |
| `9ecdf5c8` | Added timeout for TCP calls such as connect, send, recv (#3432) | PORTED |
| `3edc8036` | Run tests in the order they are declared in the source file. (#3451) | N/A (Catch2 test registration order; optional) |
| `f1d748e5` | Remove the fileapi.h include in os-inl.h (#3444) | PORTED (`src/details/os_windows.cpp` — drop redundant `#include <fileapi.h>`; `windows_include.h` / `windows.h` sufficient for `FlushFileBuffers`) |
| `4418909a` | Bump fmt to 12.0.0 | N/A (fmt **12.x** — **5A**) |
| `1bea38ed` | clang-format | N/A (formatting only) |
| `486b5555` | Version 1.16.0 | N/A (v1.x release version bump) |
| `dd3ca04a` | set CMAKE_BUILD_TYPE only in top-level project (#3480) | PORTED |
| `3f7e5028` | fix sign-compare warning (#3479) | PORTED (`os_windows.cpp` `utf8_to_wstrbuf` — `assert` vs `static_cast<int>(target.size())`) |
| `88a0e07a` | Change access scope for ANSI target_file_ from private to protected (#3486) | PORTED (`include/spdlog/sinks/ansicolor_sink.h`) |
| `cdbd64e2` | Fix sign conversion warnings in qt_sinks.h (#3487) | PORTED (`include/spdlog/sinks/qt_sinks.h` — `qsizetype` + `colors_.at(static_cast<size_t>(msg.log_level))`) |
| `8806ca65` | Fix UWP detection. (#3489) | PORTED (`src/details/os_windows.cpp` `getenv` — `WINAPI_FAMILY` vs `WINAPI_FAMILY_DESKTOP_APP`) |
| `6004e3d1` | Fix issue #3483 (#3491) | PORTED (`SPDLOG_NO_TZ_OFFSET` CMake option; `z_formatter`; `utc_minutes_offset` stubs; tests; v1 `test_stdout_api` extra case N/A) |
| `b3688ba1` | Set IndentPPDirectives to "None" on clang-format | N/A (formatting / tooling only) |
| `ea3e747e` | Bump fmt to 12.1.0 | N/A (same — **5A**) |
| `c5061bb9` | Update LICENSE file | N/A (license text may differ; reconcile at release if needed) |
| `878ad2e3` | Supress MSVC C4834 warning triggeed by fmt 12.1.0 | N/A (MSVC C4834 + fmt **12.1** — **5A** with fmt bump) |
| `2c1eafc8` | Backport warning fix from fmt head | N/A (same) |
| `3f03542d` | Remove warning 4834 suppression | N/A (same) |
| `d2100d5d` | Fix: include <fcntl.h> in tcp_client.h to avoid compilation failures on Unix (#3497) | PORTED |
| `0209b12c` | tests: fix unit tests to not be affected by custom level names (#3492) | PORTED (`tests/includes.h` — `#undef` custom level name macros before spdlog includes) |
| `32dd298d` | Docs: fix misleading comment in blocking_queue header (#3504) | PORTED (`include/spdlog/details/mpmc_blocking_q.h` file header; per-method comments already correct) |
| `09a674b7` | Fix %z when pattern_type_type is utc - should be +00:00 | PORTED |
| `2670f47d` | Fix warning | SUPERSEDED (`z_formatter` in `pattern_formatter.cpp`: `SPDLOG_NO_TZ_OFFSET` vs UTC `+00:00` structure already matches v1 fix) |
| `b656d1ce` | Windows utc_minutes_offset(): Fix historical DST accuracy and improve offset calculation speed (~2.5x) (#3508) | PORTED |
| `79524ddd` | spdlog version 1.17.0 | N/A (v1.x release version bump) |
| `6b240a89` | Replace C-style cast with reinterpret_cast in udp_client (#3509) | PORTED (`udp_client_unix.h` `send` — `reinterpret_cast`, `sizeof(sockAddr_)`) |
| `33375433` | fix: initialize null_atomic_int::value to zero (#3513) | PORTED (`null_mutex.h` — `null_atomic::value{}` value-initializes `T`) |
| `d299603e` | Add missing const qualifiers to reference variables (#3514) | N/A (v1 `registry-inl` + MDC `pattern_formatter`; v2 has no registry/MDC there — dup_filter covered by `1774e700`) |
| `1774e700` | Add const qualifier to get_time_ and filter_ member functions (#3515) | PORTED (`dup_filter_sink::filter_` const + `const` duration; `get_time_` already `const` in v2) |
| `309204d5` | Rename local variables to avoid shadowing member functions (#3516) | PORTED (`logger::should_flush` uses `flush_level()`; `daily_file_sink` `new_filename` locals) |
| `f2a9dec0` | Fix function arguments names different warnings (#3519) | PORTED (`spdlog::should_log(level log_level)`; v1 cfg/async renames N/A on v2) |
| `687226d9` | The upd_sink and dist_sink files have been modified to address Passed by value warnings. (#3520) | PORTED (`udp_sink` takes `const udp_sink_config &`; `dist_sink` ctor already `std::move(sinks)`) |
| `472945ba` | Fix shadow member warning in example file (#3521) | PORTED (`example/example.cpp` `my_type` `value_` / ctor param) |
| `6c5d6329` | Fix should_log comment (#3534) | SUPERSEDED (v2 `logger.h` already has “return true if logging is enabled…”) |
| `566b2d14` | Fix #3525: Make level name matching case-insensitive (#3535) | PORTED |
| `fc7e9c87` | Update common-inl.h | SUPERSEDED (v2 has no `common-inl.h`; `src/common.cpp` includes `<cctype>` for `level_from_str`) |
| `c49c7cf9` | Allow empty DEBUG_POSTFIX property in CMakeLists (#3530) | PORTED (`DEBUG_POSTFIX "${SPDLOG_DEBUG_POSTFIX}"` — empty cache value allowed) |
| `1685e694` | Fix deprecated copy constructor usage of fmt::format_string (#3541) | SUPERSEDED (v2 `logger::log_with_format_` takes `const format_string_t &` and uses `fmt::vformat_to`; no `details::to_string_view(fmt)` path) |
| `d5af52d9` | Fix format_string propagation (#3543) | SUPERSEDED (v2 `logger`/`spdlog` already forward `format_string_t` to `log_with_format_`) |
| `0f7562a0` | tests: timezone: Provide DST rules when setting TZ on POSIX systems (#3542) | PORTED |
| `1fbc60a5` | docs: fix SPDLOG_LEVEL env example (#3561) | N/A (docs-only) |
| `45b67eee` | Add constructor for dup_filter_sink with sinks parameter (#3549) | PORTED |
| `3c61b051` | [ci] Update actions/checkout to latest major relese (#3575) | PORTED |
