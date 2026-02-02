#!/usr/bin/env python3
"""
Test ground truth parameters to verify line straightness
Manually loads GT, constructs params, and evaluates cost
"""

import subprocess
import json
import sys

def main():
    dataset = "share/calibration/test1"

    # Load ground truth
    with open(f"{dataset}/ground_truth.json", 'r') as f:
        gt = json.load(f)

    print("="*80)
    print("TEST 1: Ground Truth Parameters")
    print("="*80)
    print(f"GT: a={gt['a']:.2f}, c={gt['c']:.2f}, d={gt['d']:.2f}")
    print(f"    cx={gt['cx']:.2f}, cy={gt['cy']:.2f}")
    print(f"    k1={gt['k1']:.4f}, k2={gt['k2']:.4f}")
    print()

    # Manually set project.json state to GT params
    with open(f"{dataset}/project.json", 'r') as f:
        project = json.load(f)

    # Create a temporary state file with GT params
    state = {
        "schema_version": 1,
        "eye_dist": gt['eye_dist_mm'],
        "yaw_l": gt['left_yaw'],
        "pitch_l": gt['left_pitch'],
        "roll_l": gt['left_roll'],
        "yaw_r": gt['right_yaw'],
        "pitch_r": gt['right_pitch'],
        "roll_r": gt['right_roll'],
        "fov_deg": 2.0 * 57.2958 * (0.5 * gt['width'] / gt['a']),  # atan approximation
        "lens_f": gt['focal_f'],
        "lens_cx": gt['cx'],
        "lens_cy": gt['cy'],
        "lens_k1": gt['k1'],
        "lens_k2": gt['k2'],
        "barrel_strength": 0,
        "preview_extrinsics": False,
        "preview_intrinsics": True,
        "compare_ga_result": False,
        "ga_phase": 1,
        "ga_population": 10,
        "ga_generations": 2,
        "ga_bounds": {
            "yaw_deg": 45.0,
            "pitch_deg": 25.0,
            "roll_deg": 45.0,
            "fov_min": 80.0,
            "fov_max": 160.0,
            "cx_delta": 60.0,
            "cy_delta": 60.0,
            "k1_min": -1.5,
            "k1_max": 0.0,
            "k2_min": -1.0,
            "k2_max": 1.0
        },
        "ga_use_trimmed_loss": True,
        "ga_trim_percent": 15.0,
        "ga_use_all_frames": True,
        "distance_weight": 0.1,
        "huber_px": 2.0,
        "huber_m": 0.03,
        "lock_distortion": False,
        "verbose_math_log": False,
        "compare_basic_params": False,
        "stage_c_enabled": False,
        "stage_c_compare": False,
        "stage_c_mode": 0,
        "max_dyaw": 3.0,
        "max_dpitch": 2.0,
        "max_droll": 3.0,
        "lambda": 0.1,
        "view_mode": 0,
        "overlay_eyes": False,
        "alpha": 50,
        "overlay_swap": False,
        "show_difference": False,
        "show_epipolar": False,
        "tint_overlay": False,
        "show_crosshair": False,
        "tool_mode": 0,
        "calibration_state": 0,
        "stage_b_diag": {
            "init_reproj": 0,
            "final_reproj": 0,
            "init_dist": 0,
            "final_dist": 0,
            "iters": 0
        },
        "stage_c_diag": {
            "dyaw": 0,
            "dpitch": 0,
            "droll": 0,
            "cost0": 0,
            "cost1": 0
        },
        "last_ga_diagnostics": {
            "best_cost": 0,
            "initial_cost": 0,
            "cost_improvement_ratio": 0,
            "num_matches_used": 0,
            "num_frames_used": 0,
            "mean_reproj_error_px": 0,
            "median_reproj_error_px": 0,
            "max_reproj_error_px": 0
        }
    }

    # Save state with GT params
    project_data = {
        "frames": project['frames'],
        "state": state
    }

    import tempfile
    import os
    temp_dir = tempfile.mkdtemp()
    temp_project = os.path.join(temp_dir, "project.json")

    with open(temp_project, 'w') as f:
        json.dump(project_data, f)

    # Copy captures
    import shutil
    captures_src = f"{dataset}/captures"
    captures_dst = os.path.join(temp_dir, "captures")
    shutil.copytree(captures_src, captures_dst)

    print("Running GA with GT parameters (pop=10, gen=2) to see trace...")
    print()

    cmd = [
        "./bin/StereoCalibrationTool",
        temp_dir,
        "--ga_run",
        "--phase", "intrinsics",
        "--ga-population=10",
        "--ga-generations=2",
        "--verbose"
    ]

    result = subprocess.run(cmd, capture_output=True, text=True)

    # Print output
    for line in result.stdout.split('\n'):
        if 'LINE STRAIGHTNESS' in line or 'Point' in line or 'Params:' in line or 'Line fit:' in line or 'Derived:' in line:
            print(line)

    # Clean up
    shutil.rmtree(temp_dir)

    print()
    print("="*80)
    print("Expected: theta should increase monotonically along the line")
    print("         (proving the line straightens with correct params)")
    print("="*80)

if __name__ == "__main__":
    main()
