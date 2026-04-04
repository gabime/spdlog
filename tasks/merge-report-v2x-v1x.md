# Merge report: v2.x ↔ v1.x integration (draft)

**Branch:** `integration/v2-sync-v1` (from `origin/v2.x`)  
**PRD:** `tasks/prd-v2x-v1x-feature-parity.md`  
**Date:** 2026-04-04

## Summary

A direct `git merge origin/v1.x` into the integration branch was **attempted** and **aborted** due to **extensive conflicts**, including many **modify/delete** cases: files **removed or reorganized on v2.x** were **modified on v1.x** after the merge-base. A one-shot textual merge is not automatable without a **file-by-file strategy** aligned with v2.x’s layout (PRD **2A**: preserve v2 public API and structure).

## Merge-base

- **SHA:** `b9cb721b9276cc93bf43910e65b1c047b3d707b5`
- **Commits on v1.x not in v2.x:** 245 (`git rev-list --count origin/v2.x..origin/v1.x`)
- **Commits on v2.x not in v1.x:** 464 (`git rev-list --count origin/v1.x..origin/v2.x`)

## Merge attempt (2.1)

- **Command:** `git merge origin/v1.x -m "Merge branch 'v1.x' into integration/v2-sync-v1 (v1.x feature parity)"`
- **Result:** Automatic merge failed; conflicts across CMake, workflows, README, benches, and a large fraction of `include/spdlog/**` and `tests/**`.
- **Notable pattern:** `CONFLICT (modify/delete)` — v2 **deleted** paths (e.g. split headers, removed `*-inl.h`, relocated or dropped translation units) while v1.x **continued to patch** those paths. Resolving by “taking v1” would **reintroduce v1 file tree** and violate v2 architecture; resolving by “taking v2” drops v1 fixes until **re-applied** against v2 files.

## Recommended next steps (hybrid 1D)

1. **Keep** `integration/v2-sync-v1` tracking v2.x (this branch is pushed to `origin`).
2. **Do not** rely on a single blind merge commit; use **topical ports**:
   - Group `v1-commit-inventory.txt` by area (fmt, CI, sinks, OS, tests).
   - For each v1 commit, either cherry-pick onto `integration/v2-sync-v1` and resolve in the **v2** file set, or **manually port** the diff onto the v2-equivalent file.
3. **fmt / CMake (5A):** After substantive code ports, align **bundled fmt** and **CMake** warning policy with **current v1.x** explicitly (may be one coordinated PR).
4. **Triage:** See **`tasks/v1-triage-complete.md`** — all 245 v1-only SHAs listed with **PORTED**, **PENDING**, or **N/A** (merge commits). Remaining **PENDING** rows must be resolved to **ported**, **superseded**, or **N/A** with rationale (PRD **3A**).

## Conflict resolutions

*(None from a merge commit — merge was aborted.)*

## Ports landed (manual / topical)

| v1.x commit | Summary | v2.x location |
|-------------|---------|---------------|
| `09a674b7` | `%z` with `pattern_time_type::utc` must print `+00:00` (not local offset). | `src/pattern_formatter.cpp` — `z_formatter` takes `pattern_time_type`; early return for UTC. |
| `b656d1ce` | Windows `utc_minutes_offset()` via `mktime` / `_mkgmtime`; pattern tests + dedicated timezone tests. | `src/details/os_windows.cpp`; `tests/test_pattern_formatter.cpp`; `tests/test_timezone.cpp` (new); `tests/CMakeLists.txt`; `include/spdlog/details/os.h` (doc comment). |
| `0f7562a0` | POSIX `TZ` strings with explicit DST rules in `test_timezone` (avoids musl / impl-defined behavior). | `tests/test_timezone.cpp` |
| `d2100d5d` | `#include <fcntl.h>` for Unix TCP client (v1 had `tcp_client.h`; v2 uses `tcp_client_unix.h`). | `include/spdlog/details/tcp_client_unix.h` |
| `3c61b051` | GitHub Actions `actions/checkout@v6` (Node deprecation). | `.github/workflows/linux.yml`, `macos.yml`, `windows.yml` |
| `9ecdf5c8` | Optional connect timeout (non-blocking + `select`); `SO_RCVTIMEO` / `SO_SNDTIMEO` when `timeout_ms` is positive; `tcp_sink_config::timeout_ms` + ctor overload. | `tcp_client_windows.h`, `tcp_client_unix.h`, `tcp_sink.h` |
| `45b67eee` | `dup_filter_sink` constructor taking `std::vector<std::shared_ptr<sink>>`. | `include/spdlog/sinks/dup_filter_sink.h` |
| `847db337` | `dup_filter_sink`: drop ctor `notification_level`; “Skipped …” uses level of last duplicate (`msg.log_level`). | `include/spdlog/sinks/dup_filter_sink.h`, `tests/test_dup_filter.cpp` |
| `8806ca65` | UWP / non-desktop: `getenv` unsupported — detect via `WINAPI_FAMILY` vs `WINAPI_FAMILY_DESKTOP_APP` (not `__cplusplus_winrt` alone). | `src/details/os_windows.cpp` |
| `e3f5a4fe` | *(SUPERSEDED)* `FMT_LIB_EXPORT` on spdlog when shared — v2 links `fmt::fmt`; fmt CMake owns exports. | — |
| `e655dbb6` | *(SUPERSEDED)* `#3408` trim `fmt.h` includes — v2 has no `spdlog/fmt/fmt.h`; `common.h` includes `fmt/base.h`. | — |
| `548b2642` | *(N/A)* MSVC C4530 / `_HAS_EXCEPTIONS` with no-exceptions — v2 has no `SPDLOG_NO_EXCEPTIONS` option in CMake. | — |
| `ae1de0dc` | *(N/A)* `load_env_levels("VAR")` — no `spdlog/cfg/` on v2. | — |
| `566b2d14` | Case-insensitive `level_from_str` (env / argv level names). | `src/common.cpp`; `tests/test_misc.cpp` |
| `dd3ca04a` | Set default `CMAKE_BUILD_TYPE` only when spdlog is the top-level CMake project. | `CMakeLists.txt` |
| `d5af52d9` | *(SUPERSEDED)* format_string propagation — v2.x already uses unified `format_string_t` forwarding to `log_with_format_`. | — |
| `32dd298d` | Misleading `enqueue_nowait` comment in blocking queue header. | `include/spdlog/details/mpmc_blocking_q.h` (file-level header; method comments already matched behavior). |
| `6c5d6329` | *(SUPERSEDED)* `should_log` doc comment — v2.x already says “if logging is enabled”. | — |
| `2aa8b6c9` | *(SUPERSEDED)* `file_helper::write` null `fd_` — already guarded in v2 `file_helper.cpp`. | — |
| `0209b12c` | Tests immune to custom `SPDLOG_LEVEL_NAMES` / `SPDLOG_SHORT_LEVEL_NAMES` in tweakme.h. | `tests/includes.h` |
| `33375433` | Value-initialize `null_atomic::value` (was `null_atomic_int` on v1). | `include/spdlog/details/null_mutex.h` |
| `6b240a89` | `sendto` / `reinterpret_cast` in UDP client. | `include/spdlog/details/udp_client_unix.h` |
| `88a0e07a` | `ansicolor_sink::target_file_` **protected** for subclasses (e.g. stdout/stderr split). | `include/spdlog/sinks/ansicolor_sink.h` |
| `7a950e02` `d343d413` `ebfa9069` `9edab1b5` | MSVC `/utf-8`: cache option **`SPDLOG_MSVC_UTF8`** (default ON) and apply only when `$<CXX_COMPILER_ID:MSVC>`. | `CMakeLists.txt` |
| `2c76e610` | *(SUPERSEDED)* abbreviated September **Sep** not **Sept** — v2 `pattern_formatter.cpp` already uses `"Sep"`. | — |
| `f1d748e5` | Drop redundant `#include <fileapi.h>` when `windows.h` already provides APIs (older SDK compatibility). | `src/details/os_windows.cpp` (v1 had `os-inl.h`) |
| `7cbf2a69` | `ansicolor_sink`: public `set_color_mode` locks; internal `set_color_mode_` must not re-lock (v1 #3323). | `src/sinks/ansicolor_sink.cpp` — removed nested `lock_guard` in `set_color_mode_` (avoids deadlock). |
| `e593f669` | *(SUPERSEDED)* `-Wextra-semi` / stray `;` after `}` — v2 `bench`, `example`, `callback_sink`, `msvc_sink` already clean. | — |
| `ad725d34` | `std::getenv` in `spdlog::details::os::getenv` (MSVC: suppress C4996; UWP: empty). | `src/details/os_windows.cpp`, `src/details/os_unix.cpp` |
| `677a2d93` | Less flaky stopwatch tests (longer sleep). | `tests/test_stopwatch.cpp` |
| `a45c9390` | *(SUPERSEDED)* `stopwatch::elapsed_ms()` — already on v2. | — |
| `eeb22c13` | *(SUPERSEDED)* `syslog_sink` virtual `syslog_prio_from_level` — already on v2. | — |
| `5673e9e5` `fe4f9952` | *(SUPERSEDED)* UTF-8 → UTF-16 conversion behavior + tests — v2 `os_windows.cpp` / `test_misc.cpp`. | — |
| `3f7e5028` | `assert` sign-compare in `utf8_to_wstrbuf` (`int` vs `size_t`). | `src/details/os_windows.cpp` |
| `a6215527` | `ringbuffer_sink(0)` throws; test expects `spdlog_ex` (v2 uses `drain` API, not v1 `last_formatted`). | `include/spdlog/sinks/ringbuffer_sink.h`, `tests/test_ringbuffer_sink.cpp` |
| `287333ee` | *(SUPERSEDED)* drop `final` from color sinks — v2 headers differ from v1 patch context. | — |
| `4397dac5` | `SPDLOG_DEBUG_POSTFIX` CMake cache (override debug library name suffix). | `CMakeLists.txt` — default `-${MAJOR}.${MINOR}d` (same as previous v2 hardcode). |
| `c49c7cf9` | Allow **empty** debug postfix (quoted property). | Same `CMakeLists.txt` change as `4397dac5`. |
| `ae525b75` | Self-contained `circular_q.h` (include `common.h`). | `include/spdlog/details/circular_q.h` |
| `2d5179ba` | *(SUPERSEDED)* `syslog_prio_from_level` visibility — v2 already `protected`/`virtual`. | — |
| `8b331e2c` | *(SUPERSEDED)* systemd `TID` — v2 uses `msg.thread_id`. | — |
| `479a5ac3` | *(SUPERSEDED)* Apple `pthread_threadid_np` availability — v2 `os_unix.cpp` already has the guarded block. | — |
| `f24f7fa2` | *(SUPERSEDED)* `#include <mutex>` for `base_sink` — v2 `src/sinks/base_sink.cpp` already includes it. | — |
| `2169a6f6` | *(SUPERSEDED)* `lock_guard` vs `unique_lock` in `mpmc_blocking_q` — v2 already uses `lock_guard` where applicable. | — |
| `271f0f3b` | *(SUPERSEDED)* daily/hourly `max_files` doc note — v2 headers already document; duplicate line removed in `daily_file_sink.h`. | — |
| `8979f7fb` | *(SUPERSEDED)* Windows `path_exists` / UTF-8 — v2 uses `std::filesystem::exists` in `os_filesystem.cpp`. | — |
| `a3a0c9d6` | *(SUPERSEDED)* `-Wsuggest-override` — v2 `base_sink.h` uses `final override`. | — |
| `5ebfc927` | *(SUPERSEDED)* `/Zc:__cplusplus` / `/MP` only for real MSVC — v2 already gates on `CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"`. | — |
| `94a8e87c` | *(SUPERSEDED)* `#3079` `create_dir` on drive-letter paths — v2 uses `std::filesystem::create_directories`. | — |
| `7cb90d1a` | *(N/A)* MSVC flags when `SPDLOG_NO_EXCEPTIONS` — v2 CMake has no such option yet. | — |
| `d387fdf9` | *(SUPERSEDED)* MINGW + `CMAKE_CXX_EXTENSIONS` — already enabled for `MINGW` in `CMakeLists.txt`. | — |
| `4052bc06` `819eb27c` `23587b0d` | *(N/A)* `registry::get` / `registry-inl.h` — v2.x uses a global logger API; no v1-style registry. | — |
| `134f9194` | *(N/A)* registry formatting — no `registry.h` on v2. | — |
| `3403f278` | *(N/A)* `set_default_logger` registry behavior — v2 `set_global_logger` / PRD **2A** surface. | — |
| `b6eeb736` `e5865186` | *(N/A)* Logger-registration callbacks + immediate revert — no persistent v1 behavior. | — |
| `ddce4215` | *(N/A)* `tweakme.h` in fmt headers — v2 has no `tweakme.h`. | — |
| `5532231b` `37b84769` | *(N/A)* Registry `string_view` accessors / revert — no v1-style registry on v2. | — |
| `3b4c775b` | *(N/A)* Comment on `set_default_logger` — registry-only. | — |
| `c9ce17ab` | *(N/A)* `INSTALL.md` — file not present on v2. | — |
| `2969dde4` | *(N/A)* Revert fmt bump — handled with `d8e0ad46` under **5A** / fmt alignment. | — |
| `6766f873` | *(SUPERSEDED)* Drop `AnalyzeTemporaryDtors` from `.clang-tidy` — v2 config never had it. | — |
| *(batch)* | *(N/A)* **Clang-format / mass reformat** (e.g. `9e36a158` … `b3688ba1`, `22b0f4fc`, `1bea38ed`) — style-only; no per-commit functional port. | — |
| *(batch)* | *(N/A)* **Logo / sponsor image** commits — v2 assets differ. | — |
| *(batch)* | *(N/A)* **v1.x version bumps** (`7c02e204`, `238c9ffa`, `2122eb21`, `8e561337`, `ac432c36`, `48bcf39a`, `486b5555`, `6fa36017`, `79524ddd`) — v2 versioning is separate. | — |
| `508d20f0` | *(N/A)* `.git-blame-ignore-revs` — optional. | — |
| `0c4fb032` | *(N/A)* `SPDLOG_CONSTEXPR_FUNC` / `FMT_CONSTEXPR` — v2 macro model differs (**5A**). | — |
| `f4afd81c` | *(N/A)* trivial v1 `common.h` edit. | — |
| `d3730937` `a5cfbf36` | *(N/A)* `FMT_UNICODE` CMake + revert — triage under **5A**. | — |
| `15f53968` | *(SUPERSEDED)* `null_sink` `final` — already `final` in v2. | — |
| `51a0deca` | *(SUPERSEDED)* duplicate `daily_file_sink` comment line — already addressed on integration branch. | — |
| `66ac83e7` `a34e08c7` | *(SUPERSEDED)* `.gitignore` entries — v2 `.gitignore` already ignores `/.vs`, `/out/build`, `/CMakeSettings.json`. | — |
| *(batch)* | *(N/A)* **README / wiki / doc-link** updates — v2 docs maintained separately. | — |
| *(batch)* | *(N/A)* **Legacy v1 CI** (`ci.yml`, Windows CI WIP, Coverity) — v2 uses `.github/workflows`. | — |
| *(batch)* | *(N/A)* **CMake comment-only** / trivial one-line CMake / formatted `CMakeLists` — no behavior port. | — |
| `943fcbd7` | *(N/A)* “Register replace logger” — registry API. | — |
| `27cb4c76` | *(N/A)* MDC readme example — v2 has no MDC stack yet. | — |
| `3edc8036` | *(N/A)* Catch2 test declaration order — optional. | — |
| `c5061bb9` | *(N/A)* `LICENSE` text update — reconcile at release if needed. | — |

**Full SHA list:** [`v1-triage-complete.md`](v1-triage-complete.md).

**Validation:** `ctest` Release on Windows — all unit tests passed after these ports.

## Integration branch

- **Remote:** `origin/integration/v2-sync-v1` (pushed; open PR against `v2.x` when ready.)

## Sign-off

- [ ] Maintainer review of this report and updated strategy.
