# GDB Crash Test Script (Event Based)
# Verifies that GDB integration can catch a crash using event callbacks.

import sys

mock_ai("Debug this", "I will start the debugger and catch the crash.")

def on_crash():
    print("EVENT: Crash detected via callback!")
    
    print("Step 5: Verifying Call Stack...")
    
    # Robust expansion loop
    for i in range(3):
        expanded = False
        all_elements = find_all()
        for item in all_elements:
            p = str(item.path)
            if "/Open" in p and not item.checked:
                if "DebugWorkspaceLayout" in p or "CallStack" in p:
                    print("Expanding: " + p)
                    item.click()
                    wait_time(0.5)
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
        _exit(0) # Exit successfully
    else:
        if found_main:
            print("? Partial Success: main found, but crash_me missing.")
        else:
            print("✗ Failure: No relevant frames found in call stack.")
        
        # Dump for debugging
        for item in all_elements:
            if "CallStack" in str(item.path):
                print("  " + str(item.path))
                
        _exit(1)

bind_event("crash", on_crash)

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
    wait_time(2)
    cont_btn = find("Continue")

if not cont_btn:
    print("Error: Continue button not found")
    sys.exit(1)

cont_btn.click()
print("Waiting for crash event...")

# Loop wait instead of hard sleep
for i in range(20):
    wait_time(1)
    # The callback will exit() if successful

print("Timeout waiting for crash event.")
sys.exit(1)