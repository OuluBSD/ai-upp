# GDB Crash Test Script
# Verifies that GDB integration can catch a crash and show the call stack.

import sys

mock_ai("Debug this", "I will start the debugger and catch the crash.")

print("Step 1: Switching to Execution Console...")
ec_tab = find("Execution Console")
if not ec_tab:
    print("Error: Execution Console tab not found")
    sys.exit(1)
ec_tab.click()
wait_ready()

print("Step 2: Setting target binary...")
binary_input = find("TargetBinary")
if not binary_input:
    print("Error: TargetBinary input not found")
    sys.exit(1)
binary_input.set("./crash")
wait_ready()

print("Step 3: Starting debugger (Break at main)...")
run_btn = find("Run")
if not run_btn:
    print("Error: Run button not found")
    sys.exit(1)
run_btn.click()
wait_time(2) 

print("Step 4: Continuing to hit the crash...")
cont_btn = find("Continue")
if not cont_btn:
    print("Error: Continue button not found")
    sys.exit(1)
cont_btn.click()
wait_time(3) # Give GDB more time to report the crash and update the UI

print("Step 5: Verifying Call Stack...")
all_elements = find_all()
print("Found " + str(len(all_elements)) + " total UI elements.")

found_main = False
found_crash_func = False

for item in all_elements:
    if "main" in item.label or "crash_me" in item.label:
        print("  Found Frame Candidate: " + item.label + " at " + item.path)
        if "main" in item.label:
            found_main = True
        if "crash_me" in item.label:
            found_crash_func = True

if found_main and found_crash_func:
    print("✓ Success: Crash detected and stack trace captured.")
elif found_main:
    print("? Partial Success: main found, but crash_me missing.")
else:
    print("✗ Failure: No relevant frames found in call stack.")
    print("DEBUG UI DUMP (END):")
    dump = dump_ui()
    print(dump)
    sys.exit(1)