#!/usr/bin/env python3
import argparse
import os
import subprocess
import sys
from pathlib import Path

def collect_tests(root: Path, pattern: str):
    tests = []
    for p in root.rglob(pattern):
        if p.is_file():
            tests.append(p)
    return sorted(tests)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin", default="bin/ModelerApp")
    ap.add_argument("--root", default="uppsrc/ModelerApp/tests")
    ap.add_argument("--pattern", default="*.py")
    ap.add_argument("--filter", default="", help="substring filter for test path")
    ap.add_argument("--stop-on-fail", action="store_true")
    args = ap.parse_args()

    root = Path(args.root)
    if not root.exists():
        print(f"Test root not found: {root}")
        return 2

    tests = collect_tests(root, args.pattern)
    if args.filter:
        tests = [t for t in tests if args.filter in str(t)]

    if not tests:
        print("No tests found")
        return 1

    failures = 0
    for t in tests:
        cmd = [args.bin, "--test", str(t)]
        print("==>", " ".join(cmd))
        proc = subprocess.run(cmd)
        if proc.returncode != 0:
            failures += 1
            print(f"FAIL: {t}")
            if args.stop_on_fail:
                break
        else:
            print(f"PASS: {t}")

    print(f"Done. total={len(tests)} failures={failures}")
    return 0 if failures == 0 else 1

if __name__ == "__main__":
    sys.exit(main())
