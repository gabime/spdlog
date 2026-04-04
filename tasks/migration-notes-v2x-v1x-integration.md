# Migration and release notes: v1.x parity on v2.x (`integration/v2-sync-v1`)

**Audience:** Downstream developers building or packaging spdlog from the **`integration/v2-sync-v1`** branch (hybrid port of applicable **v1.x** fixes into the **v2.x** layout).  
**PRD:** [`prd-v2x-v1x-feature-parity.md`](prd-v2x-v1x-feature-parity.md) — API intent **2A**, dependency baseline **5A**, full commit triage **3A**.

**Status:** This branch is an **integration line** toward **`v2.x`**; it is not a tagged release. Rebase or fast-forward relative to **`origin/v2.x`** may change; pin a commit SHA for reproducible builds.

---

## 1. Dependencies (5A)

| Item | Requirement / note |
|------|-------------------|
| **{fmt}** | Default build **fetches fmt [12.1.0](https://github.com/fmtlib/fmt/releases/tag/12.1.0)** via CMake (`cmake/fmtlib.cmake`). Same bundled **`FMT_VERSION`** as current **`origin/v1.x`** (120100). |
| **External fmt** | Set **`SPDLOG_FMT_EXTERNAL=ON`** and install **fmt 12.x** (CMake package **`find_package(fmt 12)`** — see `cmake/spdlogConfig.cmake.in`). Older **fmt 11** installs are **not** satisfied by the installed config. |
| **CMake** | **3.23+** (project minimum). |
| **C++** | **C++17** or newer (project default). |

**MSVC:** The bundled **fmt** target may be built with **`/wd4834`** (workaround for a **C4834** diagnostic in fmt **12.1.0**’s `locale_ref` with the stock tarball). This is scoped to the **fmt** target, not your whole project.

---

## 2. Public API (2A)

The integration branch **preserves the v2.x public surface**: compiled library layout, **`include/spdlog/`** structure, **`async_sink`**-based async logging, and **no** v1-only headers such as `spdlog/cfg/*.h`, **`mdc.h`**, or the v1 **registry** API.

**Not ported as v1-shaped APIs** (by design; see [`v1-triage-complete.md`](v1-triage-complete.md)):

- **Mapped Diagnostic Context (MDC)** — v1 **MDC** commits are triaged **N/A** on this v2 tree.
- **v1 `async_logger` / `thread_pool`** — v2 uses **`async_sink`** and **`mpmc_blocking_q`**; v1 async/thread-pool–specific behavior is **N/A**.
- **`spdlog/cfg`**, **`load_env_levels`**, **registry** helpers that exist only on v1 — **N/A** where v2 uses **`set_global_logger`** / different registration model.

If you rely on those v1 APIs, stay on **v1.x** or plan an explicit port; this branch does not reintroduce the v1 file tree.

---

## 3. CMake options (high-signal)

New or particularly relevant options on this branch (non-exhaustive; see top-level **`CMakeLists.txt`**):

| Option | Purpose |
|--------|---------|
| **`SPDLOG_FMT_EXTERNAL`** | Use system **fmt** instead of FetchContent (**12.x**). |
| **`SPDLOG_SANITIZE_THREAD`** / **`SPDLOG_SANITIZE_ADDRESS`** | Thread / address sanitizer for **tests** (and **spdlog** library when enabled); **mutually exclusive**. CI may pass **`SPDLOG_SANITIZE_THREAD`** on Linux Clang. |
| **`SPDLOG_NO_TZ_OFFSET`** | Platforms without reliable `tm_gmtoff` / offset: **`%z`** placeholder and UTC offset helpers degrade gracefully. |
| **`SPDLOG_WCHAR_CONSOLE`** (Windows) | Optional **UTF-8 → wide** path for **`wincolor_sink`** (**`WriteConsoleW`**). |
| **`SPDLOG_MSVC_UTF8`** | **`/utf-8`** on real MSVC for **fmt** / UTF-8 literals (default **ON**). |
| **`SPDLOG_DEBUG_POSTFIX`** | Cache string for **`DEBUG_POSTFIX`** (may be empty). |

---

## 4. Behavioral and portability fixes (from v1.x ports)

These are **bug fixes / parity** items you may notice when upgrading from an older **v2.x** snapshot (see [`merge-report-v2x-v1x.md`](merge-report-v2x-v1x.md) and [`commits-ported.txt`](commits-ported.txt)):

- **Timezones:** **`%z`** with **`pattern_time_type::utc`**, Windows **`utc_minutes_offset`**, POSIX **`TZ`** tests — see **`test_timezone.cpp`** / **`test_pattern_formatter.cpp`**.
- **Sinks / I/O:** **`rotating_file_sink::rotate_now`** mutex; **`basic_file_sink::truncate()`**; **`dup_filter_sink`** ctor / notification level behavior; TCP **timeouts** on **`tcp_sink`**; UDP **`const udp_sink_config&`** ctor.
- **OS:** **`getenv`** via **`std::getenv`** (MSVC / UWP guards), **`utf8_to_wstrbuf`** fixes, **`udp_client_unix`** **`reinterpret_cast`** for **`sockaddr`**.
- **Tests / CI:** **`actions/checkout@v6`**; optional **TSAN** matrix on Linux.

No separate guarantee of **ABI** stability across pre-release SHAs of this branch (**2A** targets **source**-level v2.x API preservation).

---

## 5. Upgrade checklist

1. **Bump fmt** to **12.x** if you use **`SPDLOG_FMT_EXTERNAL`** (or drop external and use the default FetchContent pin).  
2. **Re-run CMake** in a clean build directory; resolve any **`find_package(spdlog)`** / **`find_package(fmt)`** version errors.  
3. **Remove** includes of v1-only headers if you were experimenting with v1 APIs.  
4. **Run** your tests plus **`ctest`** for spdlog if you vendor it.  
5. **Track** [`merge-report-v2x-v1x.md`](merge-report-v2x-v1x.md) for the full port list and rationale.

---

## 6. References

- Full v1-only commit triage: [`v1-triage-complete.md`](v1-triage-complete.md)  
- Merge / port report: [`merge-report-v2x-v1x.md`](merge-report-v2x-v1x.md)  
- Landed port SHAs (commentary): [`commits-ported.txt`](commits-ported.txt)
