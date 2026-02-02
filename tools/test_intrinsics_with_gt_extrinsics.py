#!/usr/bin/env python3
"""
Test intrinsics recovery when extrinsics are initialized to GT
This tests if the intrinsics solver works when pose is known
"""

import subprocess
import json
import sys
import os
import shutil
import tempfile

def main():
    dataset = "share/calibration/test1"

    # Load ground truth
    with open(f"{dataset}/ground_truth.json", 'r') as f:
        gt = json.load(f)

    # Load project
    with open(f"{dataset}/project.json", 'r') as f:
        project = json.load(f)

    print("="*80)
    print("TEST: Intrinsics Recovery with GT Extrinsics")
    print("="*80)
    print(f"GT Intrinsics: f={gt['focal_f']:.2f}, k1={gt['k1']:.4f}, k2={gt['k2']:.4f}")
    print(f"GT Extrinsics L: yaw={gt['left_yaw']:.2f}, pitch={gt['left_pitch']:.2f}, roll={gt['left_roll']:.2f}")
    print(f"GT Extrinsics R: yaw={gt['right_yaw']:.2f}, pitch={gt['right_pitch']:.2f}, roll={gt['right_roll']:.2f}")
    print()

    # Create state with GT extrinsics but wrong intrinsics
    state = {
        "schema_version": 1,
        "eye_dist": gt['eye_dist_mm'],
        "yaw_l": gt['left_yaw'],  # GT!
        "pitch_l": gt['left_pitch'],  # GT!
        "roll_l": gt['left_roll'],  # GT!
        "yaw_r": gt['right_yaw'],  # GT!
        "pitch_r": gt['right_pitch'],  # GT!
        "roll_r": gt['right_roll'],  # GT!
        "fov_deg": 90.0,  # Wrong initial guess
        "lens_f": 0,
        "lens_cx": 0,
        "lens_cy": 0,
        "lens_k1": 0,  # Wrong initial guess
        "lens_k2": 0,  # Wrong initial guess
        "barrel_strength": 0,
        "preview_extrinsics": False,
        "preview_intrinsics": True,
        "compare_ga_result": False,
        "ga_phase": 1,
        "ga_population": 200,
        "ga_generations": 300,
        "ga_bounds": {
            "yaw_deg": 45.0,
            "pitch_deg": 25.0,
            "roll_deg": 45.0,
            "fov_min": 80.0,
            "fov_max": 150.0,
            "cx_delta": 60.0,
            "cy_delta": 60.0,
            "k1_min": -1.5,
            "k1_max": 0.5,  # Allow positive too
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
        "stage_b_diag": {"init_reproj": 0, "final_reproj": 0, "init_dist": 0, "final_dist": 0, "iters": 0},
        "stage_c_diag": {"dyaw": 0, "dpitch": 0, "droll": 0, "cost0": 0, "cost1": 0},
        "last_ga_diagnostics": {
            "best_cost": 0, "initial_cost": 0, "cost_improvement_ratio": 0,
            "num_matches_used": 0, "num_frames_used": 0,
            "mean_reproj_error_px": 0, "median_reproj_error_px": 0, "max_reproj_error_px": 0
        }
    }

    # Save to temp directory
    temp_dir = tempfile.mkdtemp()
    temp_project_file = os.path.join(temp_dir, "project.json")

    project_data = {
        "frames": project['frames'],
        "state": state
    }

    with open(temp_project_file, 'w') as f:
        json.dump(project_data, f)

    # Copy captures
    captures_src = f"{dataset}/captures"
    captures_dst = os.path.join(temp_dir, "captures")
    shutil.copytree(captures_src, captures_dst)

    print("Running GA intrinsics with GT extrinsics...")
    print()

    cmd = [
        "./bin/StereoCalibrationTool",
        temp_dir,
        "--ga_run",
        "--phase", "intrinsics",
        "--ga-population=200",
        "--ga-generations=300",
        "--verbose"
    ]

    result = subprocess.run(cmd, capture_output=True, text=True)

    # Print last 40 lines
    lines = result.stdout.split('\n')
    for line in lines[-40:]:
        print(line)

    # Clean up
    shutil.rmtree(temp_dir)

    print()
    print("="*80)
    print("If this works, intrinsics solver is OK but needs good extrinsics init")
    print("="*80)

if __name__ == "__main__":
    main()
