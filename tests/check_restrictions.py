#!/usr/bin/env python3
"""Small source restriction checker for Problem 6.

This is intentionally narrow. It checks only the body of multiply_by_24.
It is not a general C parser and is not intended as a security boundary.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path


def strip_comments_and_literals(source: str) -> str:
    pattern = re.compile(
        r"""
        //.*?$                         |
        /\*.*?\*/                     |
        "(?:\\.|[^"\\])*"             |
        '(?:\\.|[^'\\])*'
        """,
        re.MULTILINE | re.DOTALL | re.VERBOSE,
    )
    return pattern.sub(" ", source)


def function_body(source: str, function_name: str) -> str:
    match = re.search(rf"\b{re.escape(function_name)}\s*\([^;]*?\)\s*\{{", source)
    if not match:
        raise ValueError(f"could not find definition of {function_name}")

    open_brace = source.find("{", match.start())
    depth = 0

    for index in range(open_brace, len(source)):
        char = source[index]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[open_brace + 1 : index]

    raise ValueError(f"unterminated body for {function_name}")


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: check_restrictions.py PATH_TO_ASSIGNMENT_C", file=sys.stderr)
        return 2

    path = Path(sys.argv[1])
    source = strip_comments_and_literals(path.read_text())
    body = function_body(source, "multiply_by_24")

    forbidden_symbols = {
        "*": "multiplication",
        "+": "addition",
        "/": "division",
        "%": "remainder",
    }

    failures: list[str] = []

    for symbol, name in forbidden_symbols.items():
        if symbol in body:
            failures.append(f"forbidden {name} operator {symbol!r}")

    forbidden_words = [
        "for",
        "while",
        "do",
        "goto",
    ]

    for word in forbidden_words:
        if re.search(rf"\b{word}\b", body):
            failures.append(f"forbidden control-flow word {word!r}")

    # Reject calls such as helper(x). Parenthesized return expressions such as
    # "return (x << 5)" are not calls.
    call_like_names = re.findall(r"\b([A-Za-z_]\w*)\s*\(", body)
    non_call_keywords = {"return", "sizeof", "_Alignof"}
    actual_calls = [
        name for name in call_like_names if name not in non_call_keywords
    ]
    if actual_calls:
        failures.append(
            "function call found: " + ", ".join(sorted(set(actual_calls)))
        )

    if failures:
        print("Problem 6 restriction check failed:", file=sys.stderr)
        for failure in failures:
            print(f"  - {failure}", file=sys.stderr)
        return 1

    print("PASS: Problem 6 operator restriction")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
