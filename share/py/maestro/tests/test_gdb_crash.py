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
wait_time(3) 

print("Step 4: Continuing to hit the crash...")
cont_btn = find("Continue")
if not cont_btn:
    # Maybe it's still starting?
    wait_time(2)
    cont_btn = find("Continue")

if not cont_btn:
    print("Error: Continue button not found")
    sys.exit(1)

cont_btn.click()
print("Waiting for crash (10s)...")
wait_time(10) # Give GDB plenty of time to report the crash and update the UI

print("Step 5: Verifying Call Stack...")

# Expand everything in trees under DebugWorkspaceLayout
for i in range(3): # Try multiple passes of expansion
    expanded = False
    all_elements = find_all()
    for item in all_elements:
        p = str(item.path)
        if "/Open" in p and not item.checked:
            if "DebugWorkspaceLayout" in p or "CallStack" in p:
                print("Expanding node: " + p)
                item.click()
                wait_time(1)
                expanded = True
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
    
    if "CallStack" in p:
        if "main" in l_low or "crash_me" in l_low or "main" in p.lower() or "crash_me" in p.lower():
            print("  Found Frame Candidate: [" + l + "] at " + p)
            if "main" in l_low or "main" in p.lower():
                found_main = True
            if "crash_me" in l_low or "crash_me" in p.lower():
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
    
    print("\nDEBUG: System Console output:")
    sc_tab = find("System Console")
    if sc_tab:
        sc_tab.click()
        wait_ready()
        # Find the RichTextCtrl under System Console. 
        # Since we don't have a direct 'get_text' yet for RichText in all versions of the walker,
        # we might just hope the logs are in find_all if it's exposed.
        # Actually, RichText doesn't expose its content to Access yet.
    
    sys.exit(1)