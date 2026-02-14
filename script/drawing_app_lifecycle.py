# SmallGuiAppForDrawing Lifecycle Automation
import subprocess
import os
import time

MAESTRO_CLI = "/home/sblo/.cache/upp.out/MaestroCLI/CLANG.Debug.Debug_Full.Gui.Main.Shared/MaestroCLI"

def log(msg):
    print("[Lifecycle] " + msg)

def maestro_call(cmd, args=[]):
    full_cmd = [MAESTRO_CLI, cmd] + args
    log(f"Running: {' '.join(full_cmd)}")
    result = subprocess.run(full_cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error: {result.stderr}")
    else:
        print(result.stdout)
    return result.returncode == 0

def run():
    pkg = "SmallGuiAppForDrawing"
    target_dir = "examples/" + pkg
    
    log("Step 1: Project Initialization")
    maestro_call("init", ["--dir", target_dir, "--name", pkg, "--force"])
    
    # Move into project directory for subsequent commands
    os.chdir(target_dir)
    
    log("Step 2: Runbook Resolution")
    maestro_call("runbook", ["resolve", "Drawing app with pencil and color tools. Must have a canvas, a toolbar with pencil and save buttons, and a color selector."])
    
    log("Step 3: Constraint Derivation")
    maestro_call("runbook", ["derive-constraints"])
    
    log("Step 4: Synthesis Planning (Phase 3)")
    maestro_call("automation", ["plan", pkg])
    
    log("Step 5: Enactment (Autonomous Execution)")
    maestro_call("automation", ["enact", pkg])
    
    log("Step 6: Formal Verification (Phase 4)")
    maestro_call("automation", ["run", pkg])
    
    log("Lifecycle Complete.")

if __name__ == "__main__":
    run()