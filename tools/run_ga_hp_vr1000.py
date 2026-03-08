import subprocess
import sys
import os
import re

def run_command(cmd):
    print(f"Running: {' '.join(cmd)}")
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    best_count = 0
    last_cost = float('inf')
    found_invalid = False
    last_lypr = None
    last_rypr = None
    
    while True:
        line = process.stdout.readline()
        if not line and process.poll() is not None:
            break
        if line:
            line_strip = line.strip()
            print(line_strip)
            
            # Check for NaN/Inf in NEW BEST lines or summary
            if "NEW BEST" in line_strip or "Best Results" in line_strip or "Focal:" in line_strip:
                if "nan" in line_strip.lower() or "inf" in line_strip.lower():
                    print(f"ERROR: Invalid value detected in line: {line_strip}")
                    found_invalid = True
            
            # Parse symmetry for extrinsics
            if "L_y/p/r:" in line_strip:
                m = re.search(r"L_y/p/r:\s*([\d\.-]+)/([\d\.-]+)/([\d\.-]+)\s*R_y/p/r:\s*([\d\.-]+)/([\d\.-]+)/([\d\.-]+)", line_strip)
                if m:
                    last_lypr = [float(x) for x in m.groups()[0:3]]
                    last_rypr = [float(x) for x in m.groups()[3:6]]

            if "NEW BEST" in line_strip:
                best_count += 1
                try:
                    # Extract cost=...
                    parts = line_strip.split()
                    for p in parts:
                        if p.startswith("cost="):
                            cost_str = p.split("=")[1].replace(",", ".")
                            cost = float(cost_str)
                            last_cost = cost
                except Exception as e:
                    print(f"Failed to parse cost: {e}")
                    
    stderr = process.stderr.read()
    if stderr:
        print("STDERR:", stderr)
        
    return process.returncode, best_count, last_cost, found_invalid, last_lypr, last_rypr

def main():
    project = "share/calibration/hp_vr1000/"
    bin_path = "./bin/StereoCalibrationTool"
    
    # 1. Build
    print("Building...")
    build_cmd = ["python3", "script/build.py", "-j", "12", "-mc", "1", "StereoCalibrationTool"]
    if subprocess.call(build_cmd) != 0:
        print("Build failed!")
        sys.exit(1)
        
    # 2. Run GA
    cmd = [
        bin_path, 
        project, 
        "--ga_run", 
        "--phase", "both_lens_then_pose", 
        "--save",
        "--ga-population=200", 
        "--ga-generations=300", 
        "--verbose"
    ]
    
    ret, bests, final_cost, found_invalid, l_ext, r_ext = run_command(cmd)
    
    if ret != 0:
        print(f"Process exited with code {ret}")
        if ret < 0 or ret == 139: # Crash
            print("Running under GDB for backtrace...")
            gdb_cmd = ["gdb", "-q", "--batch", "-ex", "run", "-ex", "bt", "-ex", "quit", "--args"] + cmd
            subprocess.run(gdb_cmd)
        sys.exit(1)
        
    if found_invalid:
        print("FAIL: NaN or Inf detected in GA output")
        sys.exit(1)
        
    print(f"\nGA Summary: Found {bests} improvements. Final cost: {final_cost}")
    
    if l_ext and r_ext:
        print(f"Final Extrinsics L: {l_ext}")
        print(f"Final Extrinsics R: {r_ext}")
        # Verify near-symmetry: pitch and roll should be roughly same sign and magnitude (base offset)
        # and yaw should be roughly opposite (toe-out)
        # yaw_sum = l_ext[0] + r_ext[0] should be small (base_yaw * 2)
        # pitch_diff = l_ext[1] - r_ext[1] should be small (asymmetry)
        yaw_avg = (l_ext[0] + r_ext[0]) / 2.0
        pitch_diff = abs(l_ext[1] - r_ext[1])
        print(f"  Base Yaw: {yaw_avg:.2f}, Pitch Asym: {pitch_diff:.2f}")
    
    if bests < 2:
        print("FAIL: GA didn't find enough improvements")
        sys.exit(1)
        
    print("SUCCESS")

if __name__ == "__main__":
    main()
