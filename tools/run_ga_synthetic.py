#!/usr/bin/env python3
"""
Synthetic Stereo Calibration GA Validation Runner

Purpose:
  Validate the GA intrinsics solver by running it on synthetic datasets
  with known ground truth parameters.

Usage:
  python3 tools/run_ga_synthetic.py [--generate] [--seed 42]

Steps:
  1. (Optional) Generate synthetic dataset
  2. Build StereoCalibrationTool
  3. Run GA intrinsics solver
  4. Compare recovered parameters to ground truth
  5. Report pass/fail with detailed error analysis
"""

import subprocess
import sys
import os
import json
import re
import math

def run_command(cmd, description=""):
    """Run a command and capture output"""
    print(f"Running: {' '.join(cmd)}")
    if description:
        print(f"  ({description})")

    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    stdout_lines = []
    while True:
        line = process.stdout.readline()
        if not line and process.poll() is not None:
            break
        if line:
            stdout_lines.append(line.strip())
            print(line.strip())

    stderr = process.stderr.read()
    if stderr:
        print("STDERR:", stderr)

    return process.returncode, stdout_lines, stderr

def parse_ga_output(stdout_lines):
    """Parse GA output to extract final parameters"""
    params = {}

    for line in stdout_lines:
        # Look for "Best Results:" section
        if "Best Results:" in line:
            continue

        # Parse focal and distortion: "Focal: 123.45, k1=-0.123, k2=0.456"
        m = re.search(r"Focal:\s*([\d\.-]+),\s*k1=([\d\.-]+),\s*k2=([\d\.-]+)", line)
        if m:
            params['focal_f'] = float(m.group(1))
            params['k1'] = float(m.group(2))
            params['k2'] = float(m.group(3))

        # Parse yaw: "Yaw L/R: -12.345 / 12.345"
        m = re.search(r"Yaw\s+L/R:\s*([\d\.-]+)\s*/\s*([\d\.-]+)", line)
        if m:
            params['left_yaw'] = float(m.group(1))
            params['right_yaw'] = float(m.group(2))

    return params

def load_ground_truth(dataset_path):
    """Load ground truth parameters from dataset"""
    gt_path = os.path.join(dataset_path, "ground_truth.json")

    if not os.path.exists(gt_path):
        print(f"ERROR: Ground truth file not found: {gt_path}")
        return None

    with open(gt_path, 'r') as f:
        gt = json.load(f)

    return gt

def compare_params(recovered, gt, tolerances):
    """Compare recovered params to ground truth and report errors"""
    errors = {}
    passed = True

    print("\n" + "="*80)
    print("COMPARISON: Recovered vs Ground Truth")
    print("="*80)

    for param, tolerance in tolerances.items():
        if param not in recovered:
            print(f"  {param:12s}: NOT RECOVERED")
            errors[param] = {'error': 'missing', 'passed': False}
            passed = False
            continue

        rec_val = recovered[param]
        gt_val = gt[param]

        # Compute error
        if 'yaw' in param or 'pitch' in param or 'roll' in param:
            # Angular params: use absolute difference
            error = abs(rec_val - gt_val)
        elif param == 'focal_f':
            # Focal: use relative error (percentage)
            error = abs((rec_val - gt_val) / gt_val) * 100.0
        else:
            # k1, k2, cx, cy: use absolute difference
            error = abs(rec_val - gt_val)

        # Check tolerance
        param_passed = error <= tolerance['max']
        passed = passed and param_passed

        status = "PASS" if param_passed else "FAIL"

        # Format output
        if 'yaw' in param or 'pitch' in param or 'roll' in param:
            print(f"  {param:12s}: GT={gt_val:8.3f}°  Rec={rec_val:8.3f}°  Err={error:7.3f}°  Tol=±{tolerance['max']:5.2f}°  [{status}]")
        elif param == 'focal_f':
            print(f"  {param:12s}: GT={gt_val:8.2f}px Rec={rec_val:8.2f}px Err={error:6.2f}%   Tol=±{tolerance['max']:4.1f}%   [{status}]")
        else:
            print(f"  {param:12s}: GT={gt_val:7.4f}   Rec={rec_val:7.4f}   Err={error:7.4f}   Tol=±{tolerance['max']:6.4f}   [{status}]")

        errors[param] = {
            'gt': gt_val,
            'recovered': rec_val,
            'error': error,
            'tolerance': tolerance['max'],
            'passed': param_passed
        }

    print("="*80)
    return passed, errors

def main():
    import argparse
    parser = argparse.ArgumentParser(description='Validate GA intrinsics solver with synthetic data')
    parser.add_argument('--generate', action='store_true', help='Generate new synthetic dataset')
    parser.add_argument('--seed', type=int, default=42, help='Random seed for generation')
    parser.add_argument('--dataset', default='share/calibration/test1', help='Dataset path')
    parser.add_argument('--population', type=int, default=200, help='GA population size')
    parser.add_argument('--generations', type=int, default=300, help='GA generations')
    parser.add_argument('--phase', default='joint', choices=['intrinsics', 'extrinsics', 'both_lens_then_pose', 'joint'], help='GA phase')
    args = parser.parse_args()

    dataset_path = args.dataset

    # Step 1: Generate dataset if requested
    if args.generate:
        print("\n" + "="*80)
        print("STEP 1: Generating Synthetic Dataset")
        print("="*80)

        # Build generator
        ret, _, _ = run_command(
            ["python3", "script/build.py", "-j", "12", "GenerateStereoCalibSynth"],
            "Building generator"
        )
        if ret != 0:
            print("ERROR: Failed to build generator")
            sys.exit(1)

        # Generate dataset
        ret, _, _ = run_command(
            ["./bin/GenerateStereoCalibSynth", "--out", dataset_path, "--seed", str(args.seed)],
            "Generating synthetic data"
        )
        if ret != 0:
            print("ERROR: Failed to generate dataset")
            sys.exit(1)

    # Step 2: Load ground truth
    print("\n" + "="*80)
    print("STEP 2: Loading Ground Truth")
    print("="*80)

    gt = load_ground_truth(dataset_path)
    if gt is None:
        sys.exit(1)

    print(f"  Baseline: {gt['eye_dist_mm']:.2f} mm")
    print(f"  Focal: {gt['focal_f']:.2f} px")
    print(f"  Principal Point: cx={gt['cx']:.1f}, cy={gt['cy']:.1f}")
    print(f"  Distortion: k1={gt['k1']:.4f}, k2={gt['k2']:.4f}")
    print(f"  Left Yaw: {gt['left_yaw']:.2f}°")
    print(f"  Right Yaw: {gt['right_yaw']:.2f}°")

    # Step 3: Build StereoCalibrationTool
    print("\n" + "="*80)
    print("STEP 3: Building StereoCalibrationTool")
    print("="*80)

    ret, _, _ = run_command(
        ["python3", "script/build.py", "-j", "12", "StereoCalibrationTool"],
        "Building calibration tool"
    )
    if ret != 0:
        print("ERROR: Failed to build StereoCalibrationTool")
        sys.exit(1)

    # Step 4: Run GA
    print("\n" + "="*80)
    print(f"STEP 4: Running GA {args.phase} Solver")
    print("="*80)

    cmd = [
        "./bin/StereoCalibrationTool",
        dataset_path,
        "--ga_run",
        "--phase", args.phase,
        f"--ga-population={args.population}",
        f"--ga-generations={args.generations}",
        "--verbose"
    ]

    ret, stdout_lines, stderr = run_command(cmd, f"GA solver ({args.phase})")

    if ret != 0:
        print(f"ERROR: GA solver exited with code {ret}")
        # Check for NaN/Inf
        for line in stdout_lines:
            if 'nan' in line.lower() or 'inf' in line.lower():
                print(f"  Found invalid value: {line}")
        sys.exit(1)

    # Check for NaN/Inf in output
    found_invalid = False
    for line in stdout_lines:
        if ("NEW BEST" in line or "Best Results" in line or "Focal:" in line):
            if "nan" in line.lower() or "inf" in line.lower():
                print(f"ERROR: Invalid value (NaN/Inf) in output: {line}")
                found_invalid = True

    if found_invalid:
        print("FAIL: GA produced NaN or Inf values")
        sys.exit(1)

    # Step 5: Parse results
    print("\n" + "="*80)
    print("STEP 5: Parsing Results")
    print("="*80)

    recovered = parse_ga_output(stdout_lines)

    if not recovered:
        print("ERROR: Could not parse GA output")
        sys.exit(1)

    print("Recovered parameters:")
    for key, val in recovered.items():
        print(f"  {key}: {val}")

    # Step 6: Compare to ground truth
    print("\n" + "="*80)
    print("STEP 6: Validation")
    print("="*80)

    # Define tolerances (from task spec)
    # For synthetic dataset with no noise, use tight tolerances
    tolerances = {
        'focal_f': {'max': 3.0},  # ±3% relative error
        'k1': {'max': 0.05},       # ±0.05 absolute
        'k2': {'max': 0.10},       # ±0.10 absolute
    }

    # Only check extrinsics if phase includes them
    if args.phase in ['extrinsics', 'both_lens_then_pose', 'joint']:
        tolerances['left_yaw'] = {'max': 5.0}   # ±5° (generous for now)
        tolerances['right_yaw'] = {'max': 5.0}

    passed, errors = compare_params(recovered, gt, tolerances)

    # Step 7: Final verdict
    print("\n" + "="*80)
    print("FINAL VERDICT")
    print("="*80)

    if passed:
        print("SUCCESS: All parameters within tolerance!")
        print("GA intrinsics solver is working correctly on synthetic data.")
        sys.exit(0)
    else:
        print("FAIL: Some parameters outside tolerance")
        print("\nFailed parameters:")
        for param, info in errors.items():
            if not info['passed']:
                print(f"  - {param}: error={info.get('error', 'N/A')}, tolerance={info.get('tolerance', 'N/A')}")

        print("\nPossible issues:")
        print("  1. Sign convention mismatch (distort vs undistort)")
        print("  2. Fitness function not measuring correct metric")
        print("  3. GA bounds too restrictive")
        print("  4. Projection model mismatch")
        sys.exit(1)

if __name__ == "__main__":
    main()
