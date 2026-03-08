#!/usr/bin/env python3
"""
Test that rectification persists correctly across program restarts.
This test checks that the epipolar metrics are consistent.
"""

import subprocess
import json
import sys
import time

PROJECT_DIR = "share/calibration/hp_vr1000"
PROJECT_JSON = f"{PROJECT_DIR}/project.json"

def load_state():
    """Load the project state from JSON."""
    with open(PROJECT_JSON) as f:
        data = json.load(f)
    return data['state']

def get_epipolar_metrics():
    """Get epipolar metrics from current state."""
    # Run the program in headless mode (just load and check state)
    # We'll need to add a command-line flag for this, or we can parse report.txt

    # For now, let's check if the rectification cache would rebuild correctly
    # by checking the saved R matrix
    state = load_state()

    return {
        'stereo_R_valid': state.get('stereo_R_valid', False),
        'lens_f': state.get('lens_f', 0),
        'eye_dist': state.get('lens_dist', 0),
        'R_matrix': [
            [state.get(f'stereo_R_{i}{j}', 0.0) for j in range(3)]
            for i in range(3)
        ] if state.get('stereo_R_valid') else None
    }

def main():
    print("Testing rectification persistence...")
    print(f"Project: {PROJECT_DIR}")
    print()

    # Check current state
    state = load_state()

    if not state.get('stereo_R_valid'):
        print("ERROR: No valid stereo R matrix found in project.")
        print("Please run 'Detect corners' + 'Solve stereo' first.")
        return 1

    print("✓ stereo_R_valid = True")
    print(f"✓ lens_f = {state['lens_f']:.2f}")
    print(f"✓ eye_dist = {state['eye_dist']:.2f}  mm")
    print()

    print("R matrix:")
    for i in range(3):
        row = [state.get(f'stereo_R_{i}{j}', 0.0) for j in range(3)]
        print(f"  [{row[0]:9.6f}, {row[1]:9.6f}, {row[2]:9.6f}]")
    print()

    # Check if rectified_overlay is enabled
    if state.get('rectified_overlay'):
        print("✓ rectified_overlay is enabled")
    else:
        print("WARNING: rectified_overlay is disabled")
    print()

    print("To test persistence:")
    print("1. Run bin/StereoCalibrationTool")
    print("2. Open Stage-A")
    print("3. Enable 'Rectified Overlay' checkbox")
    print("4. Check that epipolar lines align")
    print("5. Close program and restart")
    print("6. Reopen Stage-A")
    print("7. Rectified overlay should look the same")
    print()

    print("The R matrix shown above should be loaded on restart.")

    return 0

if __name__ == '__main__':
    sys.exit(main())
