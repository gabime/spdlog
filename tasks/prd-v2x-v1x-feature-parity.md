# PRD: Bring v1.x features into the v2.x line

## Introduction / Overview

The **v1.x** branch is the current main development line for spdlog and receives ongoing fixes, dependency updates (for example fmt bumps), CI changes, and feature work. The **v2.x** branch contains substantial improvements and API or architectural changes targeted at a future major line, but it has **diverged** from v1.x and no longer includes many commits that landed on v1.x after the branches split.

**Problem:** Consumers and maintainers cannot treat v2.x as “v1.x plus improvements” because v2.x is missing bug fixes, portability updates, documentation corrections, and dependency alignment that exist on v1.x.

**Goal:** Update the v2.x line so it **incorporates the relevant changes from v1.x** (up to the current v1.x branch) while **preserving v2.x’s own improvements**. The result should be a coherent v2.x that is not stale relative to v1.x for shared components (core library, sinks, tests, build/CI, docs), subject to explicit decisions on merge strategy and compatibility.

**Repository context (informational):** As of the latest fetch, **v1.x** and **origin/v2.x** share a common ancestor but have diverged: hundreds of commits are unique to each side. This is a **non-trivial port/merge**, not a fast-forward.

---

## Goals

1. **Feature and fix parity (shared surface):** v2.x should include behavior, fixes, and updates from v1.x that apply to code paths shared between the two lines (unless explicitly excluded as non-goals).
2. **Preserve v2.x value:** v2.x-specific improvements (API, structure, performance) must remain intact unless deliberately superseded by a v1.x change with team agreement.
3. **Predictable quality:** All agreed automated tests pass on supported platforms; CI workflows are updated and green for the v2.x line where applicable.
4. **Clear documentation:** Release notes or migration notes describe what was merged from v1.x and any intentional differences between v1.x and v2.x.

---

## Decisions recorded (stakeholder input)

| # | Topic | Decision |
|---|--------|----------|
| 1 | Integration strategy | **1D — Hybrid:** Merge `v1.x` into a v2-based integration branch first, resolve conflicts per **2A**, then use smaller follow-up ports (cherry-picks or PRs) for gaps, risky areas, or anything missed; maintain a full **3A** triage of `v2.x..v1.x` commits until complete. |
| 2 | API / ABI during sync | **2A** — Preserve existing v2.x public API where possible; additive or internal changes unless unavoidable. |
| 3 | Scope of v1.x port | **3A** — Full parity: everything applicable from v1.x since merge-base for shared code (with explicit triage for “superseded” / “N/A”). |
| 4 | Timeline | **4B — High priority (~one release cycle):** Treat as main-track work with milestones (merge + CI green, then triage completion, fmt/docs + release notes) before a tagged v2.x pre-release in that cycle. |
| 5 | fmt and compiler baseline | **5A** — Match current **v1.x** bundled/documented **fmt** (major/minor) and align **compiler warning** handling with v1.x for shared code unless v2.x documents a deliberate, reviewed exception. |

*Execution detail for **1**, **4**, and **5** is expanded in **Guidance and recommendations** below.*

---

## User Stories

1. **As a library maintainer**, I want v2.x to include fixes and updates from v1.x so I do not maintain two unrelated histories or re-discover the same bugs on v2.x.
2. **As a downstream developer** evaluating spdlog v2.x, I want assurance that v2.x is based on a current v1.x baseline so I am not choosing between “new API” and “recent bug fixes.”
3. **As a contributor**, I want a documented merge/port process and success criteria so I can land changes without accidental regressions or scope creep.
4. **As a CI/release owner**, I want the v2.x branch to build and test cleanly on the same (or explicitly documented) matrix as we expect for releases.

---

## Functional Requirements

1. The work must **identify** all v1.x commits not present in v2.x (from the merge-base through current v1.x) and **classify** each as: port into v2.x, already superseded on v2.x, or not applicable (v1-only code paths).
2. The system must **integrate** applicable v1.x changes into v2.x using the **hybrid strategy (1D)** (merge into an integration branch, then follow-ups as needed), resolving conflicts explicitly with recorded rationale where behavior differs between branches.
3. The **build system** (CMake, options, warnings policy) on v2.x must remain consistent with v2.x goals after merging v1.x changes; any intentional deviation from v1.x must be documented.
4. **Third-party dependencies** must follow **5A:** fmt baseline (bundled/minimum) and warning policy aligned with **current v1.x** for the integration result, with any v2.x-only exception documented and reviewed.
5. **Unit and integration tests** relevant to merged changes must pass; new tests from v1.x must be brought over when they cover shared behavior.
6. **CI configuration** (.github workflows or equivalent) must be updated so v2.x is validated appropriately (not necessarily identical to v1.x if v2.x has a different matrix by design).
7. **Documentation** (README, version macros, migration notes) must reflect the new baseline and any breaking or intentional differences between v1.x and v2.x.
8. The team must produce a **short merge report** listing major areas touched (e.g. sinks, formatting, OS-specific code, Qt/UWP, TCP timeouts) and any decisions that rejected a v1.x port.

---

## Non-Goals (Out of Scope)

1. **Redefining v2.x product vision:** This effort does not replace planning for what v2.x “should be” long-term; it only brings v1.x work forward into the existing v2.x direction unless stakeholders explicitly expand scope.
2. **Guaranteeing binary or source compatibility** with every historical v2.x snapshot (narrowed by **2A**: prioritize preserving the *current* intended v2.x public API, not every experimental pre-release snapshot).
3. **Implementing new features** beyond what is required to port v1.x commits and keep tests/CI healthy (no opportunistic refactors unrelated to the port).
4. **Retiring v1.x** or changing the default branch policy (unless the organization decides separately).

---

## Design Considerations

- Prefer **minimal behavioral drift** when resolving conflicts: if v1.x fixed a bug in shared logic, the ported v2.x code should exhibit the same fix unless v2.x intentionally changed semantics.
- **User-visible API** on v2.x should follow v2.x conventions; porting may require adapting v1.x patches to v2.x APIs rather than literal cherry-picks.
- **Documentation** should call out any v2.x-only APIs so users are not confused by v1.x docs copied verbatim.

---

## Technical Considerations

- **Divergence:** Expect a large number of commits unique to each branch; a one-shot merge may produce many conflicts. Alternatives include merging v1.x into v2.x, rebasing v2.x onto v1.x, or systematic cherry-pick/port with a tracking spreadsheet.
- **Merge-base:** Use `git merge-base v1.x v2.x` to anchor analysis; compare ranges `v2.x..v1.x` and `v1.x..v2.x` for commit lists.
- **fmt and formatting:** v1.x has moved through fmt bumps and related warning fixes; v2.x may use different constraints—resolve version and compiler warning policy explicitly.
- **Platform-specific code:** Commits touching Windows, UWP, POSIX `TZ`, TCP, etc. need validation on representative environments.
- **No implementation in this document:** This PRD describes requirements only; the team chooses concrete Git workflow and review steps.

---

## Success Metrics

1. **CI:** Required pipelines for the v2.x integration pass on the agreed platform/compiler matrix.
2. **Tests:** No new failures in the v2.x test suite attributable to the port; regressions from v1.x fixes are caught by tests brought over or added.
3. **Coverage of v1.x delta:** The merge report shows **high coverage** of v1.x commits (e.g. 100% triaged, with explicit “N/A” or “superseded” for the rest—not silent drops).
4. **Stakeholder sign-off:** At least one maintainer approves the integration strategy summary and release-note accuracy.

---

## Guidance and recommendations (items 1, 4, 5)

### 1 — Integration strategy (merge, rebase, cherry-pick, hybrid)

**Context:** Both lines have diverged heavily (hundreds of commits unique to each side). **3A** requires every applicable v1.x change to be accounted for; **2A** means conflict resolution often favors **keeping v2.x’s public headers and intended API**, while still landing v1.x **behavior fixes** in implementations.

| Approach | What it means | Strengths | Risks / costs |
|----------|----------------|-----------|----------------|
| **A — Merge `v1.x` into `v2.x`** (on a long-lived branch first, e.g. `integration/v2-sync-v1`) | One merge commit brings the full v1.x history into the v2.x line. | Single integration point; git history shows the merge; `git log v2.x..v1.x` becomes empty for content you successfully merged. | Large conflict surface in one step; review can be heavy; easy to hide mistakes without strict review. |
| **B — Rebase `v2.x` onto current `v1.x`** | Replay v2.x commits on top of v1.x tip. | Linear history; mental model “v2 is v1 + deltas.” | Can be **very** painful with hundreds of v2 commits; rewrites published history (force-push) if `v2.x` is shared; same logical conflicts as merge, but spread across many steps. |
| **C — Cherry-pick / topical batches** | Port v1.x commits (or grouped diffs) in waves: CI, bundled deps, core, sinks, etc. | Reviewable chunks; clear attribution; easier to enforce **2A** per file. | Easy to miss ordering dependencies; duplicate conflict resolution; needs a **tracking list** to reach **3A** (100% triaged). |
| **D — Hybrid** | e.g. merge `v1.x` → `v2.x` once, then cherry-picks or small PRs for fixes that were too risky in the big merge, or for follow-up gaps. | Balances speed of merge with controlled cleanup. | Requires discipline to close the “gap list” after the merge. |

**Recommendation (default):** **D — Hybrid**, implemented as:

1. **Branch:** From current `v2.x`, create `integration/v2-sync-v1` (name as you prefer).
2. **Merge `v1.x` into that branch** (not necessarily into `v2.x` directly until green). This is usually faster than rebasing hundreds of v2 commits and avoids rewriting remote history.
3. **Resolve conflicts** with explicit rules: **public API** → keep v2.x unless the v1.x change is purely additive and consistent with v2.x; **implementation / tests / CI** → prefer carrying v1.x fixes forward; document any intentional “v2 wins” behavior.
4. **Close the loop for 3A:** After the merge, run `git log` / compare ranges and ensure **every** remaining v1-only commit is marked ported, superseded-on-v2, or N/A—with reasons in the merge report.
5. **Optional:** Use **topical cherry-picks** only where the big merge was wrong or too risky to do in one shot.

**When to prefer B (rebase):** Small number of v2-only commits, or `v2.x` is not widely shared and maintainers want a linear history on top of `v1.x`. For large divergence, B is usually worse than merge + cleanup.

**When to prefer C alone:** Legal/process need for commit-by-commit review, or merge conflicts are predicted to be unreviewable as one PR—in that case, still use a **spreadsheet or checklist** keyed by commit SHA so **3A** is not missed.

---

### 4 — Timeline and release expectation

**Reality check:** Full parity (**3A**) plus API care (**2A**) across a large divergence is **not** reliably a “few days” unless one person is full-time on it and conflicts are mild.

| Option | Rough meaning | Fits when… |
|--------|----------------|------------|
| **A — Urgent (weeks)** | Hard focus; ship a first integratable branch quickly. | Dedicated owner; acceptable to merge in stages with follow-up PRs; risk tolerance for a few rough edges fixed immediately after. |
| **B — High priority (~one release cycle)** | Main track work with defined milestones before a tagged v2 pre-release. | Team wants **merge + CI green + spot-check tests** as “milestone 1,” then fmt/docs hardening as “milestone 2” in the same cycle. |
| **C — Standard / incremental** | Multiple cycles; possibly merge early, then port gaps in follow-up releases. | Limited bandwidth; **3A** still enforced via triage list over time. |
| **D — No fixed date** | Quality and completeness before any tag. | v2.x not yet promised to downstream; avoids rushing bad merges. |

**Recommendation:** **B or D** for most teams: **B** if you want a **time-boxed** integration branch and a pre-release tag; **D** if `v2.x` is still experimental and correctness matters more than a calendar. **A** is only realistic with dedicated time and acceptance of follow-up fix PRs.

**Milestone suggestion (works with B or C):**

- **M1:** Integration branch exists; merge from `v1.x` applied; **CI green** on agreed matrix; no known crashers in tests.
- **M2:** **3A** triage list 100% complete; merge report published.
- **M3:** **5A** applied (fmt/toolchain alignment with v1.x); release notes + migration notes for v2.x users.

---

### 5 — fmt and compiler baseline after sync

**Tension:** **3A** pushes you toward **behavioral** alignment with v1.x (including fixes that assume a given **fmt** API and warning set). **2A** pushes you to keep **v2.x’s public surface**—usually compatible with “same fmt major” if headers and `format_string` usage match.

| Option | Meaning | Fits when… |
|--------|---------|------------|
| **A — Match current v1.x fmt (major/minor) and warnings policy** | v2.x after sync uses the **same default bundled / documented fmt** as v1.x unless v2.x documents an override. | You want the fewest surprises when porting v1.x fixes and tests; simplest story for **3A**. |
| **B — Keep v2.x’s planned fmt/toolchain (e.g. newer than v1.x); backport fixes only** | v2.x leads on dependency version; v1.x commits adapted. | v2.x intentionally targets newer fmt/compiler; you accept extra work rewriting some v1.x patches. |
| **C — New shared baseline for both lines** | Org decision to bump fmt (and maybe compiler floor) everywhere as part of this effort. | You want one fmt story across v1.x and v2.x going forward; **larger scope** (two branches + release coordination). |

**Recommendation:** Start with **A** for the **integration branch**: match **v1.x’s fmt** and mirror its **warning-suppression / compiler** patterns for shared code, so ported tests and fixes behave the same. After the branch is stable, consider **C** as a **follow-up** coordinated release if you want both lines on a newer fmt without re-litigating the port.

**Practical “done” criteria for 5 (whichever option you pick):**

- **Single policy** in CMake/docs: bundled fmt version **or** minimum external fmt version is explicit for v2.x builds.
- **CI matrix** matches the claimed compilers (or documents exceptions).
- **No known fmt-related regressions** compared to v1.x for shared formatting paths (run tests from both lines where applicable).

---

## Open questions

*None — all listed decisions are recorded above. New questions may be added here if scope or upstream policy changes during implementation.*

---

## Related artifacts (integration branch)

For the **`integration/v2-sync-v1`** workstream: full triage [`tasks/v1-triage-complete.md`](v1-triage-complete.md), merge report [`tasks/merge-report-v2x-v1x.md`](merge-report-v2x-v1x.md), and downstream **migration / release notes** [`tasks/migration-notes-v2x-v1x-integration.md`](migration-notes-v2x-v1x-integration.md).

## Document history

| Version | Date       | Notes                    |
|---------|------------|--------------------------|
| 1.0     | 2026-04-04 | Initial draft from brief |
| 1.1     | 2026-04-04 | Recorded 2A, 3A; added guidance for 1, 4, 5 |
| 1.2     | 2026-04-04 | Recorded 1D, 4B, 5A; closed open questions |
| 1.3     | 2026-04-04 | Added “Related artifacts” (triage, merge report, migration notes) |
