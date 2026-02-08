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
wait_time(5) # Give GDB more time to report the crash and update the UI

print("Step 5: Verifying Call Stack...")

# Expand everything in trees under DebugWorkspaceLayout
while True:
    expanded = False
    all_elements = find_all()
    for item in all_elements:
        p = str(item.path)
        # Use simpler matching for tree expansion
        if "/Open" in p and not item.checked:
            if "DebugWorkspaceLayout" in p or "CallStack" in p:
                print("Expanding node: " + p)
                item.click()
                wait_time(1)
                expanded = True
                break
    if not expanded:
        break

all_elements = find_all()
print("Found " + str(len(all_elements)) + " total UI elements.")

found_main = False
found_crash_func = False

for item in all_elements:
    l = str(item.label)
    l_low = l.lower()
    p = str(item.path)
    
    # Check both label and path for the desired strings
    # We are looking for something like "Item_X_main" or "Item_X_crash_me"
    if "CallStack" in p:
        if "main" in l_low or "crash_me" in l_low:
            print("  Found Frame Candidate: [" + l + "] at " + p)
            if "main" in l_low:
                found_main = True
            if "crash_me" in l_low:
                found_crash_func = True

if found_main and found_crash_func:
    print("✓ Success: Crash detected and stack trace captured.")
else:
    if found_main:
        print("? Partial Success: main found, but crash_me missing.")
    else:
        print("✗ Failure: No relevant frames found in call stack.")
    
    print("DEBUG: Elements containing 'CallStack' in path:")
    for item in all_elements:
        p = str(item.path)
        if "CallStack" in p:
            label = str(item.label)
            checked = str(item.checked)
            print("  " + p + " (label: " + label + ", checked: " + checked + ")")
    
    # print("DUMP:")
    # print(dump_ui())
    sys.exit(1)
