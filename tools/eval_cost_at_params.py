#!/usr/bin/env python3
"""
Evaluate cost at specific parameters by setting tight bounds
"""

import subprocess
import json
import sys
import os
import shutil
import tempfile

def eval_cost(dataset, focal, cx, cy, k1, k2, description):
    """Evaluate cost at specific intrinsic parameters (with GT extrinsics)"""

    # Load ground truth for extrinsics
    with open(f"{dataset}/ground_truth.json", 'r') as f:
        gt = json.load(f)

    # Load project
    with open(f"{dataset}/project.json", 'r') as f:
        project = json.load(f)

    # Create state with specific parameters
    # Make bounds very tight around target values
    eps = 0.001
    fov_deg = 2.0 * 57.2958 * (0.5 * gt['width'] / focal)

    state = {
        "schema_version": 1,
        "eye_dist": gt['eye_dist_mm'],
        "yaw_l": gt['left_yaw'],
        "pitch_l": gt['left_pitch'],
        "roll_l": gt['left_roll'],
        "yaw_r": gt['right_yaw'],
        "pitch_r": gt['right_pitch'],
        "roll_r": gt['right_roll'],
        "fov_deg": fov_deg,
        "lens_f": focal,
        "lens_cx": cx,
        "lens_cy": cy,
        "lens_k1": k1,
        "lens_k2": k2,
        "barrel_strength": 0,
        "preview_extrinsics": False,
        "preview_intrinsics": True,
        "compare_ga_result": False,
        "ga_phase": 1,
        "ga_population": 1,  # Just one individual
        "ga_generations": 1,
        "ga_bounds": {
            "yaw_deg": 45.0,
            "pitch_deg": 25.0,
            "roll_deg": 45.0,
            "fov_min": fov_deg - eps,
            "fov_max": fov_deg + eps,
            "cx_delta": eps,
            "cy_delta": eps,
            "k1_min": k1 - eps,
            "k1_max": k1 + eps,
            "k2_min": k2 - eps,
            "k2_max": k2 + eps
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

    print(f"{description}")
    print(f"  Params: f={focal:.2f}, cx={cx:.2f}, cy={cy:.2f}, k1={k1:.4f}, k2={k2:.4f}")

    cmd = [
        "./bin/StereoCalibrationTool",
        temp_dir,
        "--ga_run",
        "--phase", "intrinsics",
        "--ga-population=1",
        "--ga-generations=1",
        "--verbose"
    ]

    result = subprocess.run(cmd, capture_output=True, text=True)

    # Find cost in output
    cost = None
    for line in result.stdout.split('\n'):
        if 'NEW BEST' in line and 'cost=' in line:
            import re
            m = re.search(r'cost=([\d\.]+)', line)
            if m:
                cost = float(m.group(1))
                print(f"  Cost: {cost:.4f}")
                break

    # Clean up
    shutil.rmtree(temp_dir)

    if cost is None:
        print("  ERROR: Could not extract cost from output")
        print("  Output:", result.stdout[-500:])

    return cost

def main():
    dataset = "share/calibration/test1"

    # Load ground truth
    with open(f"{dataset}/ground_truth.json", 'r') as f:
        gt = json.load(f)

    print("="*80)
    print("COST EVALUATION at Different Parameter Sets")
    print("="*80)
    print(f"GT: f={gt['focal_f']:.2f}, k1={gt['k1']:.4f}, k2={gt['k2']:.4f}")
    print(f"    (extrinsics fixed to GT)")
    print()

    # Test 1: Ground truth
    cost_gt = eval_cost(dataset, gt['focal_f'], gt['cx'], gt['cy'], gt['k1'], gt['k2'],
                        "1. Ground Truth")

    # Test 2: Recovered by GA (k1 positive)
    cost_rec_pos = eval_cost(dataset, 289.25, 310.83, 222.92, 0.4622, -4.4204,
                              "2. GA Recovered (k1 positive)")

    # Test 3: Same as recovered but flip k1 sign
    cost_rec_neg = eval_cost(dataset, 289.25, 310.83, 222.92, -0.4622, -4.4204,
                              "3. GA Recovered with k1 sign flipped")

    # Test 4: GT focal, GT cx/cy, but recovered k1/k2
    cost_hybrid1 = eval_cost(dataset, gt['focal_f'], gt['cx'], gt['cy'], 0.4622, -4.4204,
                              "4. GT focal + GT center + recovered k1/k2")

    # Test 5: Recovered focal/center, GT k1/k2
    cost_hybrid2 = eval_cost(dataset, 289.25, 310.83, 222.92, gt['k1'], gt['k2'],
                              "5. Recovered focal/center + GT k1/k2")

    print()
    print("="*80)
    print("SUMMARY")
    print("="*80)
    if all(c is not None for c in [cost_gt, cost_rec_pos, cost_rec_neg]):
        print(f"  GT:                     {cost_gt:10.4f}")
        print(f"  GA result (k1>0):       {cost_rec_pos:10.4f}")
        print(f"  GA result (k1<0):       {cost_rec_neg:10.4f}")
        if cost_hybrid1 is not None:
            print(f"  GT f/center + rec k:    {cost_hybrid1:10.4f}")
        if cost_hybrid2 is not None:
            print(f"  Rec f/center + GT k:    {cost_hybrid2:10.4f}")
        print()

        if cost_rec_pos < cost_rec_neg:
            print("  => Cost is LOWER with k1>0 (GA found correct local minimum)")
            print("  => This suggests the COST FUNCTION or MODEL has a sign issue!")
        else:
            print("  => Cost is LOWER with k1<0 (correct sign)")
            print("  => GA got stuck in wrong local minimum")

if __name__ == "__main__":
    main()
