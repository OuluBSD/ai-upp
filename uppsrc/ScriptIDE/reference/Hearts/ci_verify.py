import cardgame_view
import time
import sys
import main

# Initialize game logic
main.start()
cardgame_view.log("Visual CI: Game logic started.")

def verify_hearts_ui():
    cardgame_view.log("Visual CI: Starting Hearts UI verification...")
    
    # 1. Initialize logic and UI
    time.sleep(1.0)
    
    # 2. Define Visual Contract
    expected = {
        "label_self":  "You  T:0  R:+0  C:13",
        "label_left":  "West  T:0  R:+0  C:13",
        "label_top":   "North  T:0  R:+0  C:13",
        "label_right": "East  T:0  R:+0  C:13"
    }
    
    keys = ["label_self", "label_left", "label_top", "label_right"]
    
    cardgame_view.log("Visual CI: Verifying zones...")
    
    # 3. Perform OCR Comparison
    for i in range(len(keys)):
        zid = keys[i]
        cardgame_view.set_highlight(zid, True)
    
    time.sleep(0.3)
    
    import ocr_verify
    report = ocr_verify.compare(expected)
    
    # 4. Result Processing
    cardgame_view.log("Visual CI: Signal=" + report["signal"])
    
    success = True
    if not report["pass"]:
        success = False
        cardgame_view.log("Visual CI: [FAIL] Visual truth mismatch detected!")
        zones = report["zones"]
        for i in range(len(zones)):
            zone = zones[i]
            if not zone["match"]:
                zid = zone["zone_id"]
                obs = str(zone["observed"])
                exp = str(zone["expected"])
                cardgame_view.log("  Zone " + zid + ": expected '" + exp + "', got '" + obs + "'")
    else:
        cardgame_view.log("Visual CI: [PASS] All critical zones verified.")

    # Clear highlights after verification
    for i in range(len(keys)):
        zid = keys[i]
        cardgame_view.set_highlight(zid, False)

    if not success:
        cardgame_view.log("Visual CI: Exiting with error code 1")
        return False
    
    cardgame_view.log("Visual CI: UI Verification Complete.")
    return True

verify_hearts_ui()
