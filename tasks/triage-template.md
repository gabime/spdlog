# v1.x → v2.x commit triage (3A)

Use this with `v1-commit-inventory.txt` (245 commits from `origin/v2.x..origin/v1.x` as of integration start).

## Column definitions

| Column | Description |
|--------|-------------|
| **SHA** | First 8+ chars of commit hash |
| **Subject** | One-line subject from `git log` |
| **Status** | `ported` \| `superseded` \| `N/A` |
| **Notes** | File paths, rationale, link to PR |
| **Reviewer** | Initials when reviewed |

## Status criteria (1.4)

- **ported** — Behavior or equivalent change exists on the integration branch (via merge, cherry-pick, or manual re-application to v2 layout).
- **superseded** — v2.x already contains a fix or replacement for the same issue; note the v2 commit or API.
- **N/A** — Change does not apply to v2.x (e.g. v1-only file removed in v2, different subsystem, or docs-only for paths that no longer exist).

## Optional grouping (1.5)

Tag commits by area for review batches: `fmt`, `ci`, `cmake`, `sinks`, `pattern`, `os`, `tcp/udp`, `windows`, `qt`, `tests`, `docs`, `other`.

## Triage rows (fill in)

| SHA | Subject | Status | Notes | Reviewer |
|-----|---------|--------|-------|----------|
| | | | | |

*(Import from `v1-commit-inventory.txt` or track in a spreadsheet.)*
