#!/usr/bin/env python3
"""Simple bridge script for ECIM C++ <-> Python integration tests.

This script supports both communication patterns:
1) File-based: --input-file and --output-file
2) Inline JSON: --input-json (prints result to stdout if no output file)
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any, Dict


def _load_payload(input_file: str | None, input_json: str | None) -> Dict[str, Any]:
    if input_json:
        parsed = json.loads(input_json)
        if isinstance(parsed, dict):
            return parsed
        return {"value": parsed}

    if input_file:
        file_data = Path(input_file).read_text(encoding="utf-8")
        parsed = json.loads(file_data)
        if isinstance(parsed, dict):
            return parsed
        return {"value": parsed}

    return {}


def main() -> int:
    parser = argparse.ArgumentParser(description="ECIM bridge script")
    parser.add_argument("--input-file", help="Path to input JSON file")
    parser.add_argument("--output-file", help="Path to output JSON file")
    parser.add_argument("--input-json", help="Inline JSON payload")
    args = parser.parse_args()

    payload = _load_payload(args.input_file, args.input_json)
    response = {
        "ok": True,
        "mode": "file" if args.input_file else "inline",
        "received": payload,
        "summary": {
            "keys": sorted(payload.keys()),
            "keyCount": len(payload),
        },
    }

    output_text = json.dumps(response, ensure_ascii=True)
    if args.output_file:
        Path(args.output_file).write_text(output_text, encoding="utf-8")
    else:
        print(output_text)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
