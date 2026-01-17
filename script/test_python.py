#!/usr/bin/env python3
import os
import subprocess
import sys

def run_command(cmd, description):
    print(f"--- {description} ---")
    print(f"Executing: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=False)
    if result.returncode != 0:
        print(f"Error: {description} failed with exit code {result.returncode}")
        return False
    return True

def main():
    # 1. Build PythonCLI
    build_cmd = [sys.executable, "script/build.py", "-j12", "PythonCLI"]
    if not run_command(build_cmd, "Building PythonCLI"):
        sys.exit(1)

    # 2. Identify test files
    test_dir = "share/py/tests"
    if not os.path.exists(test_dir):
        print(f"Error: Test directory {test_dir} not found")
        sys.exit(1)

    test_files = sorted([f for f in os.listdir(test_dir) if f.endswith(".py")])
    
    # 3. Run tests
    cli_path = "bin/PythonCLI"
    if not os.path.exists(cli_path):
        print(f"Error: {cli_path} not found after build")
        sys.exit(1)

    failed_tests = []
    for test_file in test_files:
        path = os.path.join(test_dir, test_file)
        test_cmd = [cli_path]
        
        if test_file == "17_threading_scheduled.py":
            test_cmd += ["--threading", "scheduled"]
        elif test_file == "18_policy.py":
            test_cmd += ["--sandbox"]
            
        test_cmd.append(path)
        
        if not run_command(test_cmd, f"Running test: {test_file}"):
            failed_tests.append(test_file)

    # 4. Special Sandbox negative test
    print("\n--- Running negative Sandbox test ---")
    negative_test_path = "share/py/tests/negative_sandbox.py"
    with open(negative_test_path, "w") as f:
        f.write("import os\nos.listdir('.')\n")
    
    test_cmd = [cli_path, "--sandbox", negative_test_path]
    print(f"Executing: {' '.join(test_cmd)} (EXPECTED TO FAIL)")
    result = subprocess.run(test_cmd, capture_output=True)
    os.remove(negative_test_path)
    
    combined_output = result.stdout + result.stderr
    if result.returncode != 0 and b"PermissionError" in combined_output:
        print("Negative sandbox test passed (failed as expected with PermissionError)")
    else:
        print(f"Error: Negative sandbox test failed! returncode={result.returncode}, output={combined_output.decode()}")
        failed_tests.append("negative_sandbox_test")

    # 5. Summary
    print("\n--- Test Summary ---")
    if not failed_tests:
        print(f"ALL {len(test_files)} TESTS PASSED!")
    else:
        print(f"FAILED {len(failed_tests)}/{len(test_files)} tests:")
        for test in failed_tests:
            print(f"  - {test}")
        sys.exit(1)

if __name__ == "__main__":
    main()
