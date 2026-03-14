#!/usr/bin/env python3
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent


def run_step(cmd, description):
    print(f"--- {description} ---")
    print(f"Executing: {' '.join(str(c) for c in cmd)}")
    result = subprocess.run(cmd, cwd=REPO_ROOT)
    if result.returncode != 0:
        print(f"Error: {description} failed with exit code {result.returncode}")
        return result.returncode
    return 0


def main():
    steps = [
        ([sys.executable, "script/test_scriptcli_mcp.py"], "ScriptCLI MCP smoke/regression"),
    ]

    for cmd, description in steps:
        rc = run_step(cmd, description)
        if rc != 0:
            return rc

    print("--- Headless test summary ---")
    print("All selected headless tests passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
