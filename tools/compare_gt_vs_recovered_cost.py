#!/usr/bin/env python3
"""
Compare cost at GT parameters vs recovered parameters
"""

import subprocess
import json
import sys
import os
import shutil
import tempfile

def run_eval(dataset, state, description):
    """Run cost evaluation with given state"""
    temp_dir = tempfile.mkdtemp()
    temp_project_file = os.path.join(temp_dir, "project.json")

    # Load project
    with open(f"{dataset}/project.json", 'r') as f:
        project = json.load(f)

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

    print(f"\n{'='*80}")
    print(f"{description}")
    print(f"{'='*80}")

    cmd = [
        "./bin/StereoCalibrationTool",
        temp_dir,
        "--ga_run",
        "--phase", "intrinsics",
        "--ga-population=10",
        "--ga-generations=1",  # Just evaluate initial population
        "--verbose"
    ]

    result = subprocess.run(cmd, capture_output=True, text=True)

    # Find cost in output
    for line in result.stdout.split('\n'):
        if 'NEW BEST' in line and 'cost=' in line:
            print(line)
            # Extract cost
            import re
            m = re.search(r'cost=([\d\.]+)', line)
            if m:
                cost = float(m.group(1))
                print(f"  => COST: {cost:.4f}")
                shutil.rmtree(temp_dir)
                return cost

    shutil.rmtree(temp_dir)
    return None

def main():
    dataset = "share/calibration/test1"

    # Load ground truth
    with open(f"{dataset}/ground_truth.json", 'r') as f:
        gt = json.load(f)

    print("="*80)
    print("COST COMPARISON: GT vs Recovered Parameters")
    print("="*80)
    print(f"GT: f={gt['focal_f']:.2f}, k1={gt['k1']:.4f}, k2={gt['k2']:.4f}")
    print(f"    yaw_l={gt['left_yaw']:.2f}, yaw_r={gt['right_yaw']:.2f}")

    # Base state template
    base_state = {
        "schema_version": 1,
        "eye_dist": gt['eye_dist_mm'],
        "barrel_strength": 0,
        "preview_extrinsics": False,
        "preview_intrinsics": True,
        "compare_ga_result": False,
        "ga_phase": 1,
        "ga_population": 10,
        "ga_generations": 1,
        "ga_bounds": {
            "yaw_deg": 45.0,
            "pitch_deg": 25.0,
            "roll_deg": 45.0,
            "fov_min": 80.0,
            "fov_max": 150.0,
            "cx_delta": 60.0,
            "cy_delta": 60.0,
            "k1_min": -1.5,
            "k1_max": 0.5,
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

    # Test 1: GT parameters
    state_gt = base_state.copy()
    state_gt.update({
        "yaw_l": gt['left_yaw'],
        "pitch_l": gt['left_pitch'],
        "roll_l": gt['left_roll'],
        "yaw_r": gt['right_yaw'],
        "pitch_r": gt['right_pitch'],
        "roll_r": gt['right_roll'],
        "fov_deg": 2.0 * 57.2958 * (0.5 * gt['width'] / gt['focal_f']),
        "lens_f": gt['focal_f'],
        "lens_cx": gt['cx'],
        "lens_cy": gt['cy'],
        "lens_k1": gt['k1'],
        "lens_k2": gt['k2'],
    })

    cost_gt = run_eval(dataset, state_gt, "EVALUATING: Ground Truth Parameters")

    # Test 2: Recovered parameters (from previous test)
    state_rec = base_state.copy()
    state_rec.update({
        "yaw_l": gt['left_yaw'],  # GT extrinsics
        "pitch_l": gt['left_pitch'],
        "roll_l": gt['left_roll'],
        "yaw_r": gt['right_yaw'],
        "pitch_r": gt['right_pitch'],
        "roll_r": gt['right_roll'],
        "fov_deg": 2.0 * 57.2958 * (0.5 * gt['width'] / 289.25),  # Recovered focal
        "lens_f": 289.25,
        "lens_cx": 310.83,
        "lens_cy": 222.92,
        "lens_k1": 0.4622,  # OPPOSITE SIGN!
        "lens_k2": -4.4204,
    })

    cost_rec = run_eval(dataset, state_rec, "EVALUATING: Recovered Parameters (k1 opposite sign)")

    # Test 3: Recovered but with GT k1 sign
    state_rec_flipped = base_state.copy()
    state_rec_flipped.update({
        "yaw_l": gt['left_yaw'],
        "pitch_l": gt['left_pitch'],
        "roll_l": gt['left_roll'],
        "yaw_r": gt['right_yaw'],
        "pitch_r": gt['right_pitch'],
        "roll_r": gt['right_roll'],
        "fov_deg": 2.0 * 57.2958 * (0.5 * gt['width'] / 289.25),
        "lens_f": 289.25,
        "lens_cx": 310.83,
        "lens_cy": 222.92,
        "lens_k1": -0.4622,  # Flip sign to match GT
        "lens_k2": -4.4204,
    })

    cost_rec_flipped = run_eval(dataset, state_rec_flipped, "EVALUATING: Recovered with k1 sign flipped")

    print("\n" + "="*80)
    print("SUMMARY")
    print("="*80)
    if cost_gt and cost_rec and cost_rec_flipped:
        print(f"  GT params:             cost = {cost_gt:.4f}")
        print(f"  Recovered (k1>0):      cost = {cost_rec:.4f}  (GA converged here)")
        print(f"  Recovered (k1<0):      cost = {cost_rec_flipped:.4f}")
        print()
        if cost_rec < cost_rec_flipped:
            print("  => GA correctly found lower cost with k1>0")
            print("  => This means GT k1 sign might be wrong OR cost function has issue!")
        else:
            print("  => k1<0 has lower cost, so GA should have found it")
            print("  => GA must be getting stuck in local minimum")
    else:
        print("  Failed to extract costs from output")

if __name__ == "__main__":
    main()
