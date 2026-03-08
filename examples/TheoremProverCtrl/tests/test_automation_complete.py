# Complete Automation Test for TheoremProverCtrl
log("Starting Complete Automation Test")

def wait_for_path(path, limit):
    start = 0.0
    while start < limit:
        el = find(path)
        if el:
            return el
        wait_time(0.5)
        start = start + 0.5
    return None

def click_by_path(path):
    el = wait_for_path(path, 5.0)
    if el:
        log("Clicking:", path)
        el.click()
        return True
    log("FAILED to find:", path)
    return False

def set_by_path(path, value):
    el = wait_for_path(path, 5.0)
    if el:
        log("Setting:", path, "to", value)
        el.set(value)
        return True
    log("FAILED to find:", path)
    return False

# 1. Wait for UI to be ready
wait_time(2.0)
click_by_path("Main/Toolbar/Clear All")
wait_time(0.5)
# log("UI DUMP:\n" + dump_ui(True))

# 2. Test Basic Proof (P implies P)
set_by_path("input_val", "P implies P")
click_by_path("Main/Toolbar/Prove")
wait_time(1.0)
out = find("output_val")
if out and "Formula proven" in out.value:
    log("SUCCESS: Basic proof P implies P verified")
else:
    val = "None"
    if out:
        val = out.value
    log("FAILURE: Basic proof P implies P failed, out=" + val)

# 3. Test Axiom Addition
set_by_path("input_val", "forall x. P(x)")
click_by_path("Main/Toolbar/Add Axiom")
wait_time(0.5)
click_by_path("Main/Toolbar/List Axioms")
wait_time(0.5)
out = find("output_val")
if out and "(∀x. P(x))" in out.value:
    log("SUCCESS: Axiom verified in list")
else:
    val = "None"
    if out:
        val = out.value
    log("FAILURE: Axiom list check failed, out=" + val)

# 4. Test Proof with Axiom
set_by_path("input_val", "P(a)")
click_by_path("Main/Toolbar/Prove")
wait_time(1.0)
out = find("output_val")
if out and "Formula proven" in out.value:
    log("SUCCESS: Proof using axiom verified")
else:
    val = "None"
    if out:
        val = out.value
    log("FAILURE: Proof using axiom failed, out=" + val)

# 5. Test UGUI Constraints
click_by_path("Main/Toolbar/Check Constraints")
wait_time(0.5)
st = find("status_val")
if st and "Constraint FAILED" in st.value:
    log("FAILURE: UGUI Constraint check failed: " + st.value)
else:
    log("SUCCESS: UGUI Constraints satisfied")

log("Automation Test FINISHED")
exit(0)
