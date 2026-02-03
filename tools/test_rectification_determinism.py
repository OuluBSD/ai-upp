#!/usr/bin/env python3
"""
Automated test for rectification determinism.
Tests that rectification produces identical results after restart.
"""
import subprocess
import json
import sys
import os
import re

PROJECT_DIR = "share/calibration/hp_vr1000"
LOG_FILE = os.path.expanduser("~/.local/state/u++/log/StereoCalibrationTool.log")
BINARY = "bin/StereoCalibrationTool"

def clear_log():
    """Clear the log file."""
    if os.path.exists(LOG_FILE):
        os.remove(LOG_FILE)

def extract_r1_p1_from_log():
    """Extract R1 and P1 matrices from log file."""
    if not os.path.exists(LOG_FILE):
        return None, None

    with open(LOG_FILE, 'r') as f:
        log = f.read()

    # Find the LAST occurrence of R1 and P1 (most recent computation)
    r1_matches = re.findall(r'ComputeStereoRectification: R1.*?\n.*?\[([-\d.]+), ([-\d.]+), ([-\d.]+)\]\n.*?\[([-\d.]+), ([-\d.]+), ([-\d.]+)\]\n.*?\[([-\d.]+), ([-\d.]+), ([-\d.]+)\]', log, re.DOTALL)
    p1_matches = re.findall(r'ComputeStereoRectification: P1.*?\n.*?\[([-\d.]+), ([-\d.]+), ([-\d.]+), ([-\d.]+)\]\n.*?\[([-\d.]+), ([-\d.]+), ([-\d.]+), ([-\d.]+)\]\n.*?\[([-\d.]+), ([-\d.]+), ([-\d.]+), ([-\d.]+)\]', log, re.DOTALL)

    if not r1_matches or not p1_matches:
        return None, None

    # Get the last match
    r1 = r1_matches[-1]
    p1 = p1_matches[-1]

    r1_matrix = [
        [float(r1[0]), float(r1[1]), float(r1[2])],
        [float(r1[3]), float(r1[4]), float(r1[5])],
        [float(r1[6]), float(r1[7]), float(r1[8])]
    ]

    p1_matrix = [
        [float(p1[0]), float(p1[1]), float(p1[2]), float(p1[3])],
        [float(p1[4]), float(p1[5]), float(p1[6]), float(p1[7])],
        [float(p1[8]), float(p1[9]), float(p1[10]), float(p1[11])]
    ]

    return r1_matrix, p1_matrix

def matrices_equal(m1, m2, tolerance=1e-6):
    """Check if two matrices are equal within tolerance."""
    if m1 is None or m2 is None:
        return False

    for i in range(len(m1)):
        for j in range(len(m1[i])):
            if abs(m1[i][j] - m2[i][j]) > tolerance:
                return False
    return True

def print_matrix(name, matrix):
    """Print a matrix nicely."""
    print(f"{name}:")
    for row in matrix:
        print("  ", row)

def main():
    print("=" * 80)
    print("Rectification Determinism Test")
    print("=" * 80)
    print()

    # Check if project exists
    if not os.path.exists(f"{PROJECT_DIR}/project.json"):
        print(f"ERROR: Project not found at {PROJECT_DIR}")
        return 1

    print(f"Project: {PROJECT_DIR}")
    print(f"Binary: {BINARY}")
    print(f"Log: {LOG_FILE}")
    print()

    # Step 1: Clear log and run program in headless mode to trigger restart rectification
    print("Step 1: Testing rectification on startup (after previous calibration)...")
    clear_log()
    # TODO: Need to add headless mode that loads project and builds rectification
    # For now, we'll manually parse the log

    print("Please run the program, wait for it to load, then close it.")
    print("Press Enter when done...")
    input()

    r1_restart, p1_restart = extract_r1_p1_from_log()

    if r1_restart is None:
        print("ERROR: Could not extract R1/P1 from restart log")
        return 1

    print("Restart rectification:")
    print_matrix("R1", r1_restart)
    print_matrix("P1", p1_restart)
    print()

    # Step 2: Run solve stereo
    print("Step 2: Testing rectification after Solve Stereo...")
    print("Please press 'Solve stereo' button, then close the program.")
    print("Press Enter when done...")
    input()

    r1_solve, p1_solve = extract_r1_p1_from_log()

    if r1_solve is None:
        print("ERROR: Could not extract R1/P1 from solve log")
        return 1

    print("Solve stereo rectification:")
    print_matrix("R1", r1_solve)
    print_matrix("P1", p1_solve)
    print()

    # Step 3: Restart again and compare
    print("Step 3: Testing rectification on second restart...")
    clear_log()
    print("Please run the program again, wait for it to load, then close it.")
    print("Press Enter when done...")
    input()

    r1_restart2, p1_restart2 = extract_r1_p1_from_log()

    if r1_restart2 is None:
        print("ERROR: Could not extract R1/P1 from second restart log")
        return 1

    print("Second restart rectification:")
    print_matrix("R1", r1_restart2)
    print_matrix("P1", p1_restart2)
    print()

    # Compare
    print("=" * 80)
    print("COMPARISON RESULTS")
    print("=" * 80)

    solve_vs_restart2 = matrices_equal(r1_solve, r1_restart2) and matrices_equal(p1_solve, p1_restart2)

    if solve_vs_restart2:
        print("✓ PASS: Solve Stereo == Second Restart")
    else:
        print("✗ FAIL: Solve Stereo != Second Restart")
        print()
        print("Differences in R1:")
        for i in range(3):
            for j in range(3):
                diff = abs(r1_solve[i][j] - r1_restart2[i][j])
                if diff > 1e-6:
                    print(f"  R1[{i}][{j}]: {r1_solve[i][j]} vs {r1_restart2[i][j]} (diff: {diff})")
        print()
        print("Differences in P1:")
        for i in range(3):
            for j in range(4):
                diff = abs(p1_solve[i][j] - p1_restart2[i][j])
                if diff > 1e-6:
                    print(f"  P1[{i}][{j}]: {p1_solve[i][j]} vs {p1_restart2[i][j]} (diff: {diff})")

    print()

    return 0 if solve_vs_restart2 else 1

if __name__ == '__main__':
    sys.exit(main())
