import subprocess
import sys
import os

def run_command(cmd):
    print(f"Running: {' '.join(cmd)}")
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    best_count = 0
    last_cost = float('inf')
    
    while True:
        line = process.stdout.readline()
        if not line and process.poll() is not None:
            break
        if line:
            print(line.strip())
            if "NEW BEST" in line:
                best_count += 1
                try:
                    # Extract cost=...
                    parts = line.split()
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
        
    return process.returncode, best_count, last_cost

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
        "--ga-population=200", 
        "--ga-generations=300", 
        "--verbose"
    ]
    
    ret, bests, final_cost = run_command(cmd)
    
    if ret != 0:
        print(f"Process exited with code {ret}")
        print("Running under GDB for backtrace...")
        gdb_cmd = [
            "gdb", "-q", "--batch", 
            "-ex", "run", 
            "-ex", "bt", 
            "-ex", "quit", 
            "--args"
        ] + cmd
        subprocess.run(gdb_cmd)
        sys.exit(1)
        
    print(f"\nGA Summary: Found {bests} improvements. Final cost: {final_cost}")
    
    if bests < 2:
        print("FAIL: GA didn't find enough improvements")
        sys.exit(1)
        
    print("SUCCESS")

if __name__ == "__main__":
    main()
