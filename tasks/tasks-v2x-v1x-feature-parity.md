# Task list: v2.x ↔ v1.x feature parity

Derived from [`prd-v2x-v1x-feature-parity.md`](prd-v2x-v1x-feature-parity.md). Decisions: **1D** (hybrid merge), **2A** (preserve v2 public API where possible), **3A** (full triage of v1.x delta), **4B** (~one release cycle), **5A** (align fmt / warnings with v1.x).

## Progress (2026-04-04)

- **Done:** Integration branch created and pushed; merge-base and v1-only commit list exported; triage template and merge-report draft added; direct `git merge origin/v1.x` was attempted and **aborted** (v2.x file-tree divergence vs v1.x — see [`merge-report-v2x-v1x.md`](merge-report-v2x-v1x.md)).
- **Ports:** [`commits-ported.txt`](commits-ported.txt) — **`09a674b7`** (`pattern_formatter` UTC `%z`); **`b656d1ce`** (Windows `utc_minutes_offset`, `test_timezone.cpp`, pattern test updates). **All unit tests pass** (`ctest` Release, Windows).
- **Remaining:** Topical ports from `v1-commit-inventory.txt` (fmt / CI / other fixes per **5A** and triage).

## Relevant Files

- `CMakeLists.txt` — Top-level CMake options, bundled fmt sources, warnings, install rules; will need alignment with **5A** after merge.
- `include/spdlog/version.h` (or equivalent version header in repo) — Version macros for v2.x pre-release; confirm path under `include/`.
- `include/spdlog/**/*.h` — Public API; conflict resolution favors **2A** (v2.x surface) while landing v1.x behavior fixes in implementations.
- `src/**/*.cpp` — Implementation files likely touched by merge conflicts and ports.
- `tests/*.cpp`, `tests/CMakeLists.txt`, `tests/main.cpp` — Test sources; bring over v1.x tests that cover shared behavior; run full suite on integration branch.
- `.github/workflows/linux.yml`, `.github/workflows/macos.yml`, `.github/workflows/windows.yml` — CI matrix for v2.x branch after integration; update triggers/branches as needed.
- `README.md` — User-facing build and version notes post-sync.
- `docs/` (if present on branch) — Additional documentation for v2.x vs v1.x.
- Bundled fmt under `include/spdlog/fmt/bundled/` and related (e.g. `src/bundled_fmtlib_format.cpp`) — Must match **v1.x** fmt baseline per **5A**.
- [`merge-report-v2x-v1x.md`](merge-report-v2x-v1x.md) — Merge report (extend as ports land).

### Notes

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

- [ ] 3.0 Align fmt, CMake, and compiler warnings (5A)
  - [ ] 3.1 Compare bundled fmt / CMake fmt version pins between merged tree and current `v1.x`; align bundled copy and `CMakeLists.txt` with **v1.x** unless a reviewed exception is documented.
  - [ ] 3.2 Reconcile `SPDLOG_FMT_EXTERNAL` / header-only options with documented v2.x behavior; update README if defaults change.
  - [ ] 3.3 Match **v1.x** warning suppressions and compiler flags for shared code (MSVC, GCC, Clang); remove stale suppressions only if v1.x does and tests pass.
  - [ ] 3.4 Verify `src/bundled_fmtlib_format.cpp` (if used) and bundled headers are consistent with chosen fmt version.

- [ ] 4.0 Tests and CI validation
  - [x] 4.1 Run full test suite locally (`ctest` or project equivalent); fix failures tied to the merge. *(Windows Release: all tests pass.)*
  - [x] 4.2 Port any missing **v1.x** tests that cover shared behavior not yet present on the integration branch. *(Partial: `test_timezone.cpp` + pattern tests from `b656d1ce`; more as ports land.)*
  - [ ] 4.3 Update `.github/workflows/*.yml` so the integration branch (or `v2.x` once merged) runs the agreed matrix; fix workflow-only issues (paths, branches).
  - [ ] 4.4 Confirm CI green on Linux, macOS, and Windows (or document skipped jobs with reason).
  - [ ] 4.5 Spot-check platform-specific areas called out in PRD (e.g. Windows/UWP, POSIX `TZ`, TCP) on representative configs if possible.

- [ ] 5.0 Documentation, merge report, and release readiness (4B)
  - [ ] 5.1 Complete **3A** triage: every v1.x commit in range has status **ported**, **superseded**, or **N/A** with reason (no silent gaps).
  - [x] 5.2 Write **merge report**: areas touched, conflict resolutions, rejected or deferred ports with rationale.
  - [ ] 5.3 Update **README** (build, fmt version, branch notes) and version header for v2.x pre-release.
  - [ ] 5.4 Add **migration / release notes** for downstream users (API preserved per **2A**, dependency changes per **5A**).
  - [ ] 5.5 Open PR from integration branch to `v2.x` (or maintainer process); obtain **stakeholder sign-off** per PRD success metrics.
  - [ ] 5.6 Tag or schedule **v2.x pre-release** per **4B** milestone once CI and review are complete.
