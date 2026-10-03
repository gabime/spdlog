#!/usr/bin/env python3
"""Check that src/spdlog.cppm re-exports everything the in-scope public headers declare.

The module interface unit is a hand-maintained list of using-declarations. Nothing in
the compiler forces it to stay in sync with the headers, so a new public name lands
silently and `import spdlog;` users only find out that it is missing at their call
site. This script is the guard: it fails whenever an in-scope header grows a public
name that the module does not re-export.

It is deliberately a text scan rather than a real parse. The headers are uniformly
clang-formatted, which makes the handful of declaration shapes below tractable, and a
libclang-based AST walk would put an LLVM dependency into CI for a fairly small gain.
If the regexes ever become the bottleneck, that is the upgrade path.

False positives are handled by adding the name to ALLOW_UNEXPORTED below, with a
reason. False negatives (a name the scan does not recognise) are tolerated: this
catches drift, it does not prove completeness.

Usage: check_module_exports.py [repo_root]
"""

import re
import sys
from pathlib import Path

# Headers whose public names the module is expected to cover. Sinks with an external
# dependency are intentionally absent: they stay header-only.
IN_SCOPE_HEADERS = [
    "spdlog/async.h",
    "spdlog/async_logger.h",
    "spdlog/common.h",
    "spdlog/formatter.h",
    "spdlog/fwd.h",
    "spdlog/logger.h",
    "spdlog/mdc.h",
    "spdlog/pattern_formatter.h",
    "spdlog/spdlog.h",
    "spdlog/stopwatch.h",
    "spdlog/cfg/argv.h",
    "spdlog/cfg/env.h",
    "spdlog/cfg/helpers.h",
    "spdlog/fmt/bin_to_hex.h",
    "spdlog/sinks/base_sink.h",
    "spdlog/sinks/basic_file_sink.h",
    "spdlog/sinks/callback_sink.h",
    "spdlog/sinks/daily_file_sink.h",
    "spdlog/sinks/dist_sink.h",
    "spdlog/sinks/dup_filter_sink.h",
    "spdlog/sinks/hourly_file_sink.h",
    "spdlog/sinks/msvc_sink.h",
    "spdlog/sinks/null_sink.h",
    "spdlog/sinks/ostream_sink.h",
    "spdlog/sinks/ringbuffer_sink.h",
    "spdlog/sinks/rotating_file_sink.h",
    "spdlog/sinks/sink.h",
    "spdlog/sinks/stdout_color_sinks.h",
    "spdlog/sinks/stdout_sinks.h",
    "spdlog/sinks/ansicolor_sink.h",
    "spdlog/sinks/wincolor_sink.h",
    "spdlog/sinks/win_eventlog_sink.h",
]

# Public names the module deliberately does not re-export.
ALLOW_UNEXPORTED = {
    # internal linkage: a namespace-scope static/constexpr cannot be named by an
    # exported using-declaration
    "default_eol": "internal linkage",
    "folder_seps": "internal linkage",
    "folder_seps_filename": "internal linkage",
    "default_async_q_size": "internal linkage",
    # implementation details of win_eventlog_sink.h
    "local_alloc_t": "win_eventlog internal",
    "win32_error": "win_eventlog internal",
    "sid_t": "win_eventlog internal",
    "eventlog": "win_eventlog internal",
    # spdlog::level::level_enum enumerators are exported individually, and the
    # SPDLOG_LEVEL_* macros are macros, not names
    "level_enum": "exported explicitly inside namespace level",
    # type traits and helpers used only inside the headers
    "conditional_static_cast": "header-internal helper",
    "custom_flags": "member typedef of pattern_formatter",
    "mdc_map_t": "member typedef of mdc",
    # aliases that exist only under a configuration the scan cannot see
    "wstring_view_t": "wchar-only, exported under the wchar guard",
    "wmemory_buf_t": "wchar-only, exported under the wchar guard",
    "wformat_string_t": "wchar-only, exported under the wchar guard",
}

# Declaration shapes recognised in the headers.
PATTERNS = [
    re.compile(r"^class\s+SPDLOG_API\s+(\w+)"),
    re.compile(r"^class\s+(\w+)\s*[:{]"),
    re.compile(r"^struct\s+SPDLOG_API\s+(\w+)"),
    re.compile(r"^struct\s+(\w+)\s*[:{]"),
    re.compile(r"^enum\s+class\s+(\w+)"),
    re.compile(r"^using\s+(\w+)\s*="),
    re.compile(r"^typedef\s+.*\s(\w+);$"),
    re.compile(r"^SPDLOG_API\s+[\w:<>,\s&*]+?\b(\w+)\("),
    re.compile(r"^\[\[noreturn\]\]\s+SPDLOG_API\s+[\w:<>,\s&*]+?\b(\w+)\("),
    re.compile(r"^inline\s+[\w:<>,\s&*]+?\b(\w+)\("),
    re.compile(r"^(?:std::shared_ptr<\w+>|void|bool)\s+(\w+)\("),
]

# Names that look like declarations but are keywords/noise.
NOISE = {"if", "for", "while", "switch", "return", "operator", "SPDLOG_API"}


def collect_header_names(include_dir: Path):
    """Return {name: [headers it was seen in]} for the in-scope headers."""
    found = {}
    for rel in IN_SCOPE_HEADERS:
        path = include_dir / rel
        if not path.exists():
            print(f"warning: {rel} listed as in-scope but not found", file=sys.stderr)
            continue
        for raw in path.read_text(encoding="utf-8", errors="replace").splitlines():
            # Only column-0 declarations are namespace scope in spdlog's formatting;
            # anything indented is a class member and not the module's business.
            line = raw.rstrip()
            if not line or line[0].isspace() or line.startswith(("//", "*", "#", "}")):
                continue
            for pattern in PATTERNS:
                match = pattern.match(line)
                if not match:
                    continue
                name = match.group(1)
                if name in NOISE or name.startswith("_"):
                    break
                found.setdefault(name, []).append(rel)
                break
    return found


def collect_exported_names(cppm: Path):
    text = cppm.read_text(encoding="utf-8", errors="replace")
    names = set()
    for match in re.finditer(r"^using\s+::[\w:]*?(\w+);", text, re.MULTILINE):
        names.add(match.group(1))
    # namespace aliases and keep-alive aliases redeclared in the purview
    for match in re.finditer(r"^namespace\s+(\w+)\s*=", text, re.MULTILINE):
        names.add(match.group(1))
    return names


def main():
    root = Path(sys.argv[1] if len(sys.argv) > 1 else Path(__file__).resolve().parent.parent)
    include_dir = root / "include"
    cppm = root / "src" / "spdlog.cppm"

    if not cppm.exists():
        print(f"error: {cppm} not found", file=sys.stderr)
        return 2

    header_names = collect_header_names(include_dir)
    exported = collect_exported_names(cppm)

    missing = {
        name: headers
        for name, headers in sorted(header_names.items())
        if name not in exported and name not in ALLOW_UNEXPORTED
    }

    if not missing:
        print(f"OK: {len(exported)} exported names cover {len(header_names)} public header names")
        return 0

    print("src/spdlog.cppm is out of sync with the headers.\n", file=sys.stderr)
    for name, headers in missing.items():
        where = ", ".join(sorted(set(headers)))
        print(f"  {name}  (declared in {where})", file=sys.stderr)
    print(
        "\nAdd a matching `using ::SPDLOG_NAMESPACE::<name>;` to src/spdlog.cppm, or, if the\n"
        "name is deliberately not part of the module, add it to ALLOW_UNEXPORTED in\n"
        "scripts/check_module_exports.py with a reason.",
        file=sys.stderr,
    )
    return 1


if __name__ == "__main__":
    sys.exit(main())
