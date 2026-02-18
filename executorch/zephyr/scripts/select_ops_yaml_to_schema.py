#!/usr/bin/env python3
import argparse
from pathlib import Path

import yaml


def _extract_ops(data):
    if isinstance(data, dict) and "operators" in data:
        entries = data["operators"]
    elif isinstance(data, list):
        entries = data
    else:
        raise ValueError("Unsupported YAML format: expected 'operators' list or top-level list")

    ops = []
    for entry in entries:
        if isinstance(entry, str):
            ops.append(entry)
        elif isinstance(entry, dict):
            if "op" in entry:
                ops.append(entry["op"])
            elif "name" in entry:
                ops.append(entry["name"])
            else:
                raise ValueError(f"Unsupported operator entry: {entry}")
        else:
            raise ValueError(f"Unsupported operator entry type: {type(entry)}")
    return ops


def main():
    parser = argparse.ArgumentParser(
        description="Convert an operator list YAML into ExecuTorch ops schema format."
    )
    parser.add_argument("--input", required=True, help="Path to the input YAML file.")
    parser.add_argument("--output", required=True, help="Path to the output YAML file.")
    args = parser.parse_args()

    input_path = Path(args.input)
    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    data = yaml.safe_load(input_path.read_text())
    ops = _extract_ops(data)

    output = [{"op": op} for op in ops]
    output_path.write_text(yaml.safe_dump(output, default_flow_style=False))


if __name__ == "__main__":
    main()
