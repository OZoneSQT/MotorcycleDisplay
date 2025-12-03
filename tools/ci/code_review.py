"""Simple static checks to enforce repository coding standards during CI."""

from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

SOURCE_GLOBS = (
    "src/**/*.cpp",
    "src/**/*.hpp",
    "include/**/*.cpp",
    "include/**/*.hpp",
)

MAX_LINE_LENGTH = 140


def iter_source_files() -> list[Path]:
    files: list[Path] = []
    for pattern in SOURCE_GLOBS:
        files.extend(ROOT.glob(pattern))
    return sorted({path for path in files if path.is_file()})


def check_file(path: Path) -> list[str]:
    failures: list[str] = []
    try:
        text = path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        failures.append(f"{path}: file is not valid UTF-8")
        return failures

    for idx, line in enumerate(text.splitlines(), start=1):
        if "\t" in line:
            failures.append(f"{path}:{idx}: tab character detected")
        if line.rstrip() != line:
            failures.append(f"{path}:{idx}: trailing whitespace detected")
        if len(line) > MAX_LINE_LENGTH:
            failures.append(f"{path}:{idx}: line exceeds {MAX_LINE_LENGTH} characters")
        upper_line = line.upper()
        if "TODO" in upper_line or "FIXME" in upper_line:
            failures.append(f"{path}:{idx}: TODO/FIXME comment detected")
    return failures


def main() -> int:
    failures: list[str] = []
    for path in iter_source_files():
        failures.extend(check_file(path))

    if failures:
        print("Code review checks failed:", file=sys.stderr)
        for failure in failures:
            print(f"  - {failure}", file=sys.stderr)
        return 1

    print("Code review checks passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
