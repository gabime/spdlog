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
| `566b2d14` | Case-insensitive `level_from_str` (env / argv level names). | `src/common.cpp`; `tests/test_misc.cpp` |
| `dd3ca04a` | Set default `CMAKE_BUILD_TYPE` only when spdlog is the top-level CMake project. | `CMakeLists.txt` |
| `d5af52d9` | *(SUPERSEDED)* format_string propagation — v2.x already uses unified `format_string_t` forwarding to `log_with_format_`. | — |

**Full SHA list:** [`v1-triage-complete.md`](v1-triage-complete.md).

**Validation:** `ctest` Release on Windows — all unit tests passed after these ports.

## Integration branch

- **Remote:** `origin/integration/v2-sync-v1` (pushed; open PR against `v2.x` when ready.)

## Sign-off

- [ ] Maintainer review of this report and updated strategy.
