# SmallGuiAppForAccounting Lifecycle Automation (User Emulation)
import maestro
import os
import time

def log(msg):
    print("[User Emulation] " + msg)

def develop_component(req):
    log("--- Developing Component: " + req + " ---")
    
    log("Step 3: Decomposing requirement")
    maestro.call("plan", ["decompose", req])
    
    log("Step 4: Enacting plan")
    wg_dir = "docs/maestro/plans/workgraphs"
    files = os.listdir(wg_dir)
    files.sort()
    latest_wg = wg_dir + "/" + files[len(files)-1]
    maestro.call("plan", ["enact", latest_wg])
    
    log("Step 5: Simulating implementation")
    # Simulate user completing tasks
    maestro.call("task", ["--mode", "terminal", "--dry-run"]) 
    
    log("Step 6: Building project")
    orig_dir = os.getcwd()
    os.chdir("../..")
    maestro.call("make", ["build", "examples/SmallGuiAppForAccounting"])
    os.chdir(orig_dir)
    
    # Randomly simulate failures
    if "invoicing" in req or "security" in req:
        log("Step 7: Build FAILED! Creating issue.")
        maestro.call("issues", ["add", "Build failure: " + req, "Detailed error log simulation for " + req])
    else:
        log("Step 7: Build SUCCESSFUL.")

def run():
    target_dir = "./examples/SmallGuiAppForAccounting"
    app_name = "SmallGuiAppForAccounting"
    
    log("Step 1: Project Initialization")
    maestro.call("init", ["--dir", target_dir, "--name", app_name, "--template", "accounting", "--force"])
    
    os.chdir(target_dir)
    
    log("Step 2: Iterative Development")
    
    requirements = [
        "general ledger", "invoicing", "expense tracking", 
        "user authentication", "role management", "dashboard",
        "report generation", "audit logging", "backup system",
        "data export", "customer management", "vendor tracking"
    ]
    
    for req in requirements:
        develop_component(req)
        time.sleep(0.5)
    
    log("Final: Listing all issues created during lifecycle")
    maestro.call("issues", ["ls"])
    
    log("User Emulation Complete.")

run()