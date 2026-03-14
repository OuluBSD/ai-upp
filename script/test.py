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


def discover_plugin_ids():
    plugins_root = REPO_ROOT / "autotest" / "Plugins"
    plugin_ids = []
    if not plugins_root.exists():
        return plugin_ids
    for plugin_dir in sorted(plugins_root.iterdir()):
        tests_dir = plugin_dir / "tests"
        if plugin_dir.is_dir() and tests_dir.is_dir():
            plugin_ids.append(plugin_dir.name)
    return plugin_ids


def main():
    steps = [
        ([sys.executable, "script/test_scriptcli_mcp.py"], "ScriptCLI MCP smoke/regression"),
    ]

    for plugin_id in discover_plugin_ids():
        steps.append(
            ([str(REPO_ROOT / "bin" / "ScriptCLI"), "plugin", "test", plugin_id],
             f"ScriptCLI plugin test: {plugin_id}")
        )

    for cmd, description in steps:
        rc = run_step(cmd, description)
        if rc != 0:
            return rc

    print("--- Headless test summary ---")
    print("All selected headless tests passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
