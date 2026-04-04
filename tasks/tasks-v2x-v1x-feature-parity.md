# Task list: v2.x ↔ v1.x feature parity

Derived from [`prd-v2x-v1x-feature-parity.md`](prd-v2x-v1x-feature-parity.md). Decisions: **1D** (hybrid merge), **2A** (preserve v2 public API where possible), **3A** (full triage of v1.x delta), **4B** (~one release cycle), **5A** (align fmt / warnings with v1.x).

## Progress (2026-04-04)

- **Done:** Integration branch created and pushed; merge-base and v1-only commit list exported; triage template and merge-report draft added; direct `git merge origin/v1.x` was attempted and **aborted** (v2.x file-tree divergence vs v1.x — see [`merge-report-v2x-v1x.md`](merge-report-v2x-v1x.md)).
- **Ports:** [`commits-ported.txt`](commits-ported.txt). **Full triage (245 v1-only SHAs):** [`v1-triage-complete.md`](v1-triage-complete.md) — **PORTED**, **PENDING**, **SUPERSEDED**, or **N/A** (merge commits). **All unit tests pass** (`ctest` Release, Windows).
- **Latest pass (triage-only, no code):** Pushed as **`aeb67cfe`** on `origin/integration/v2-sync-v1`. Reclassified **15** v1-only commits that do not apply to the current v2 integration branch as written:
  - **N/A — v1 `async_logger` / `thread_pool`** (v2 uses `async_sink` + `mpmc_blocking_q`): `fe79bfcc`, `6725584e`, `ec661f98`, `a19c76a4`, `62302019` (thread-pool / async-logger behavior and tests); `16e0d2e7`, `63d18842` (flush promise / condition_variable / callback on the v1 `thread_pool`).
  - **N/A — bundled fmt 10.2.1 churn:** `d8e0ad46`, `1e7d7e07` — superseded by the current fmt pin (**5A**).
  - **N/A — revert pair:** `3c23c27d` (revert of external-fmt #3312; covered with `7f8060d5` triage).
  - **SUPERSEDED — fmt 11.x / `to_string_view` / bundle:** `faa0a7a9`, `85bdab0c` (superseded by **12.1.0** pin); `276ee5f5`, `7f8060d5`, `96a8f625` (v2 has no v1 `common.h` `to_string_view(fmt)` chain; logging uses **`log_with_format_`** / **`fmt::vformat_to`**, same story as **`1685e694`**).
  - Docs: [`v1-triage-complete.md`](v1-triage-complete.md), [`commits-ported.txt`](commits-ported.txt) comment block, [`merge-report-v2x-v1x.md`](merge-report-v2x-v1x.md) subsection **“Triage-only batch (v1 async / fmt delta vs v2)”**. Prior code work: `c3aed4b6` wchar console.
- **Triage snapshot:** **42 PORTED**, **62 SUPERSEDED**, **141 N/A**, **0 PENDING** — [`v1-triage-complete.md`](v1-triage-complete.md) (**3A** complete).
- **5A (fmt):** Bundled **fmt 12.1.0** via `cmake/fmtlib.cmake` (matches `origin/v1.x` `FMT_VERSION` **120100**); **`FMT_INSTALL ON`** for fmt 12 subproject + install export; MSVC **`/wd4834`** on target **`fmt`**; **`find_dependency(fmt 12)`** in `cmake/spdlogConfig.cmake.in`; README notes bundled vs external. Prior: **`9fe79692`** TSAN CMake; MDC / Feature 3379 **N/A**.

## Relevant Files

- `CMakeLists.txt` — Top-level CMake options, bundled fmt sources, warnings, install rules; will need alignment with **5A** after merge.
- `include/spdlog/version.h` (or equivalent version header in repo) — Version macros for v2.x pre-release; confirm path under `include/`.
- `include/spdlog/**/*.h` — Public API; conflict resolution favors **2A** (v2.x surface) while landing v1.x behavior fixes in implementations.
- `src/**/*.cpp` — Implementation files likely touched by merge conflicts and ports.
- `tests/*.cpp`, `tests/CMakeLists.txt`, `tests/main.cpp` — Test sources; bring over v1.x tests that cover shared behavior; run full suite on integration branch.
- `.github/workflows/linux.yml`, `.github/workflows/macos.yml`, `.github/workflows/windows.yml` — CI matrix for v2.x branch after integration; update triggers/branches as needed.
- `README.md` — User-facing build and version notes post-sync.
- `docs/` (if present on branch) — Additional documentation for v2.x vs v1.x.
- **`cmake/fmtlib.cmake`** — FetchContent **fmt** version (**5A**; v2.x does not vendor `include/spdlog/fmt/bundled/` like v1.x).
- [`merge-report-v2x-v1x.md`](merge-report-v2x-v1x.md) — Merge report (extend as ports land).

### Notes

- **Commits:** Prefer **small, topical commits** (code vs. doc-only when practical), **`ctest` Release** before push, then **`git push origin integration/v2-sync-v1`** so the branch and CI stay current.
- Build and test with CMake: configure a build directory, then `cmake --build .` and `ctest` (or your project’s documented test target). Example: `cmake -B build -S . -DSPDLOG_BUILD_TESTS=ON` then `cmake --build build` and `ctest --test-dir build`.
- Conflict resolution rule of thumb: **public headers** → preserve v2.x unless v1.x change is additive and compatible; **`.cpp` / tests / CI** → prefer carrying v1.x fixes forward; document intentional “v2 wins” cases in the merge report.
- Keep a **triage spreadsheet or checklist** (commit SHA → ported | superseded | N/A + reason) until **3A** is 100% complete.

## Instructions for Completing Tasks

**IMPORTANT:** As you complete each task, check it off by changing `- [ ]` to `- [x]`. Update after completing each sub-task, not only after finishing a parent task.

Example: `- [ ] 1.1 Read file` → `- [x] 1.1 Read file` (after completing).

## Tasks

- [x] 0.0 Create integration branch
  - [x] 0.1 Fetch latest `v1.x` and `v2.x` (e.g. `git fetch origin v1.x v2.x`).
  - [x] 0.2 Create branch from current `v2.x` (e.g. `git checkout -b integration/v2-sync-v1 origin/v2.x` or team naming convention).
  - [x] 0.3 Push integration branch to remote so CI and reviewers can track it (if policy allows).

- [x] 1.0 Build v1.x commit inventory and triage framework (3A)
  - [x] 1.1 Record merge-base: `git merge-base v1.x v2.x` (document SHA).
  - [x] 1.2 List commits on v1.x not in v2.x: `git log v2.x..v1.x --oneline` (export to file for tracking).
  - [x] 1.3 Create triage sheet with columns: SHA, subject, status (ported / superseded / N/A), notes, reviewer initials.
  - [x] 1.4 Define “superseded” and “N/A” criteria with the team (e.g. v1-only path removed on v2.x → N/A).
  - [x] 1.5 Optionally group commits by area (fmt, CI, sinks, OS-specific) to plan reviews.

- [ ] 2.0 Merge `v1.x` into integration branch and resolve conflicts (1D, 2A)
  - [ ] 2.1 Merge `v1.x` into the integration branch (`git merge v1.x` or merge `origin/v1.x`); resolve conflicts in working tree.
  - [ ] 2.2 For each conflicted file, apply rules: keep **v2.x public API** where required by **2A**; land **v1.x behavior fixes** in implementations and tests.
  - [ ] 2.3 Document each non-obvious resolution (short bullet in merge report draft or commit message).
  - [ ] 2.4 Build locally (Release and at least one Debug) and fix compile errors before pushing.
  - [x] 2.5 Follow-up: cherry-pick or small PRs for anything that could not be merged cleanly or was deferred (hybrid **1D**). *(Ongoing: `09a674b7`, `b656d1ce` — see [`commits-ported.txt`](commits-ported.txt).)*

- [x] 3.0 Align fmt, CMake, and compiler warnings (5A)
  - [x] 3.1 Compare bundled fmt / CMake fmt version pins between merged tree and current `v1.x`; align bundled copy and `CMakeLists.txt` with **v1.x** unless a reviewed exception is documented. *(**12.1.0** FetchContent — same `FMT_VERSION` as `origin/v1.x`.)*
  - [x] 3.2 Reconcile `SPDLOG_FMT_EXTERNAL` / header-only options with documented v2.x behavior; update README if defaults change. *(`find_dependency(fmt 12)` + README bullet.)*
  - [x] 3.3 Match **v1.x** warning suppressions and compiler flags for shared code (MSVC, GCC, Clang); remove stale suppressions only if v1.x does and tests pass. *(MSVC `/wd4834` on **`fmt`** for fmt **12.1.0** `locale_ref` / stock tarball.)*
  - [x] 3.4 Verify `src/bundled_fmtlib_format.cpp` (if used) and bundled headers are consistent with chosen fmt version. *(v2.x: FetchContent only — no in-tree bundled fmt sources.)*

- [ ] 4.0 Tests and CI validation
  - [x] 4.1 Run full test suite locally (`ctest` or project equivalent); fix failures tied to the merge. *(Windows Release: all tests pass.)*
  - [x] 4.2 Port any missing **v1.x** tests that cover shared behavior not yet present on the integration branch. *(Partial: `test_timezone.cpp` + pattern tests from `b656d1ce`; more as ports land.)*
  - [x] 4.3 Update `.github/workflows/*.yml` so the integration branch (or `v2.x` once merged) runs the agreed matrix; fix workflow-only issues (paths, branches). *(checkout@v6 on linux/macos/windows)*
  - [ ] 4.4 Confirm CI green on Linux, macOS, and Windows (or document skipped jobs with reason).
  - [ ] 4.5 Spot-check platform-specific areas called out in PRD (e.g. Windows/UWP, POSIX `TZ`, TCP) on representative configs if possible.

- [ ] 5.0 Documentation, merge report, and release readiness (4B)
  - [x] 5.1 Complete **3A** triage: every v1.x commit in range has status **ported**, **superseded**, or **N/A** with reason (no silent gaps). *([`v1-triage-complete.md`](v1-triage-complete.md): **42** / **62** / **141** / **0**; **5A** fmt **12.1.0** triage updates for former fmt-bump rows.)*
  - [x] 5.2 Write **merge report**: areas touched, conflict resolutions, rejected or deferred ports with rationale.
  - [x] 5.3 Update **README** (build, fmt version, branch notes) and version header for v2.x pre-release. *(fmt **12.1.0** / external **12.x** note in Features.)*
  - [ ] 5.4 Add **migration / release notes** for downstream users (API preserved per **2A**, dependency changes per **5A**).
  - [ ] 5.5 Open PR from integration branch to `v2.x` (or maintainer process); obtain **stakeholder sign-off** per PRD success metrics.
  - [ ] 5.6 Tag or schedule **v2.x pre-release** per **4B** milestone once CI and review are complete.
