#!/usr/bin/env python3
"""Verify that AppMetadata defaults remain aligned with the .env file and optionally update them."""

from __future__ import annotations

import argparse
import pathlib
import re
import sys
from typing import Dict, Iterable, List, Tuple, Union

ENV_TRUE_VALUES = {"debug_true", "true", "1"}
ENV_FALSE_VALUES = {"debug_false", "false", "0"}
SPECIAL_FIELD_KEYS = {"bDebugEnabled": "debug"}
STRING_FIELD_PATTERN = re.compile(r"std::string\s+(s\w+)\s*\{\s*\"([^\"]*)\"\s*\};")
BOOL_FIELD_PATTERN = re.compile(r"bool\s+(b\w+)\s*\{\s*(true|false)\s*\};")

AppDefaultValue = Union[str, bool]


class EnvSyncError(RuntimeError):
    """Raised when the sync validation cannot complete."""


def parse_env_file(path_env: pathlib.Path) -> Dict[str, str]:
    if not path_env.exists():
        raise EnvSyncError(f"Environment file not found: {path_env}")
    entries: Dict[str, str] = {}
    for raw_line in path_env.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        if "=" not in line:
            raise EnvSyncError(f"Malformed line in {path_env}: {raw_line}")
        key, value = line.split("=", 1)
        entries[key.strip().lower()] = value.strip()
    return entries


def parse_metadata_defaults(path_header: pathlib.Path) -> Dict[str, AppDefaultValue]:
    if not path_header.exists():
        raise EnvSyncError(f"Header file not found: {path_header}")
    content = path_header.read_text(encoding="utf-8")
    defaults: Dict[str, AppDefaultValue] = {}
    for match in STRING_FIELD_PATTERN.finditer(content):
        defaults[match.group(1)] = match.group(2)
    for match in BOOL_FIELD_PATTERN.finditer(content):
        defaults[match.group(1)] = match.group(2) == "true"
    if not defaults:
        raise EnvSyncError(f"No defaults discovered in {path_header}")
    return defaults


def field_to_env_key(field_name: str) -> str:
    if field_name in SPECIAL_FIELD_KEYS:
        return SPECIAL_FIELD_KEYS[field_name]
    base = field_name[1:]
    snake = re.sub(r"(?<!^)([A-Z])", r"_\1", base).lower()
    return snake


def normalize_env_value(field_name: str, env_value: str) -> AppDefaultValue:
    if field_name.startswith("b"):
        normalized = env_value.strip().lower()
        if normalized in ENV_TRUE_VALUES:
            return True
        if normalized in ENV_FALSE_VALUES:
            return False
        raise EnvSyncError(f"Unsupported boolean value '{env_value}' for field '{field_name}'")
    return env_value


def compare_defaults_to_env(
    defaults: Dict[str, AppDefaultValue],
    env_data: Dict[str, str],
) -> Tuple[List[str], List[Tuple[str, str, str, AppDefaultValue]]]:
    missing_keys: List[str] = []
    mismatches: List[Tuple[str, str, str, AppDefaultValue]] = []
    for field_name, default_value in defaults.items():
        env_key = field_to_env_key(field_name)
        if env_key not in env_data:
            missing_keys.append(env_key)
            continue
        env_value = normalize_env_value(field_name, env_data[env_key])
        if env_value != default_value:
            mismatches.append((env_key, field_name, env_data[env_key], default_value))
    return missing_keys, mismatches


def build_replacements(
    mismatches: Iterable[Tuple[str, str, str, AppDefaultValue]]
) -> Dict[str, AppDefaultValue]:
    replacements: Dict[str, AppDefaultValue] = {}
    for _, field_name, env_value, _ in mismatches:
        replacements[field_name] = normalize_env_value(field_name, env_value)
    return replacements


def apply_header_updates(path_header: pathlib.Path, replacements: Dict[str, AppDefaultValue]) -> None:
    content = path_header.read_text(encoding="utf-8")
    for field_name, replacement in replacements.items():
        if isinstance(replacement, bool):
            pattern = re.compile(rf"(bool\s+{re.escape(field_name)}\s*\{{\s*)(true|false)(\s*;\s*)")
            desired = "true" if replacement else "false"
            content, count = pattern.subn(rf"\\1{desired}\\3", content, count=1)
        else:
            str_value = replacement
            escaped = str_value.replace("\\", "\\\\").replace('"', '\\"')
            pattern = re.compile(rf"(std::string\s+{re.escape(field_name)}\s*\{{\s*\")([^\"]*)(\"\s*;\s*)")
            content, count = pattern.subn(rf"\\1{escaped}\\3", content, count=1)
        if count != 1:
            raise EnvSyncError(f"Failed to update field '{field_name}' in {path_header}")
    path_header.write_text(content, encoding="utf-8")


def main() -> int:
    script_path = pathlib.Path(__file__).resolve()
    implementation_root = script_path.parents[2]
    default_env = implementation_root / ".env"
    default_header = implementation_root / "include" / "logic" / "entities" / "AppMetadata.hpp"

    parser = argparse.ArgumentParser(description="Validate AppMetadata defaults against the .env file")
    parser.add_argument("--env", dest="env_path", default=str(default_env), help="Path to the .env file to validate")
    parser.add_argument("--header", dest="header_path", default=str(default_header), help="Path to AppMetadata.hpp")
    parser.add_argument("--update-header", action="store_true", help="Rewrite the header defaults to match the .env values")
    parser.add_argument("--quiet", action="store_true", help="Suppress success output")
    args = parser.parse_args()

    path_env = pathlib.Path(args.env_path).resolve()
    path_header = pathlib.Path(args.header_path).resolve()

    try:
        env_data = parse_env_file(path_env)
        defaults = parse_metadata_defaults(path_header)
        missing_keys, mismatches = compare_defaults_to_env(defaults, env_data)
        extra_keys = sorted(set(env_data) - {field_to_env_key(field) for field in defaults})

        if args.update_header and mismatches:
            if missing_keys:
                raise EnvSyncError("Cannot update header while required keys are missing from the .env file")
            replacements = build_replacements(mismatches)
            apply_header_updates(path_header, replacements)
            if not args.quiet:
                print(f"Updated {path_header} to match {path_env}")
            return 0

        if missing_keys or mismatches or extra_keys:
            if missing_keys:
                print("Missing keys:", ", ".join(sorted(missing_keys)), file=sys.stderr)
            if mismatches:
                for key, _, env_value, default_value in mismatches:
                    print(f"Mismatch for '{key}': env='{env_value}' default='{default_value}'", file=sys.stderr)
            if extra_keys:
                print("Unmapped keys:", ", ".join(extra_keys), file=sys.stderr)
            return 1

        if not args.quiet:
            print("All AppMetadata defaults match the .env values.")
        return 0
    except EnvSyncError as exc:
        print(f"verify_env_sync: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
