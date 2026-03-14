import ocr_verify
import cardgame_view
import time
import sys

def verify_hearts_ui():
    cardgame_view.log("Visual CI: Starting Hearts UI verification...")
    
    # 1. Initialize logic and UI
    # In ScriptIDE, this script runs in the context of the active game view.
    # We assume 'start()' has been called in main.py already if running via Execute.
    
    # Give it a moment to render
    time.sleep(0.5)
    
    # 2. Define Visual Contract
    # Based on PLAYER_NAMES = ["You", "West", "North", "East"]
    # and update_hud formatting: name + "  T:" + str(state.scores[i]) + "  R:+" + str(state.round_scores[i]) + "  C:" + str(len(state.players[i]))
    # For a fresh game (Round 1, Phase PASSING, 13 cards each):
    expected = {
        "label_self":  "You  T:0  R:+0  C:13",
        "label_left":  "West  T:0  R:+0  C:13",
        "label_top":   "North  T:0  R:+0  C:13",
        "label_right": "East  T:0  R:+0  C:13"
    }
    
    cardgame_view.log(f"Visual CI: Verifying {len(expected)} zones...")
    
    # 3. Perform OCR Comparison
    report = ocr_verify.compare(expected)
    
    # 4. Result Processing
    cardgame_view.log(f"Visual CI: Signal={report['signal']} (Total ms: {report.get('total_ms', 'N/A')})")
    
    success = True
    if not report["pass"]:
        success = False
        cardgame_view.log("Visual CI: [FAIL] Visual truth mismatch detected!")
        for zone in report["zones"]:
            if not zone["match"]:
                zid = zone["zone_id"]
                obs = zone["observed"]
                exp = zone["expected"]
                cardgame_view.log(f"  Zone {zid}:")
                cardgame_view.log(f"    Expected: '{exp}'")
                cardgame_view.log(f"    Observed: '{obs}'")
                cardgame_view.log(f"    Hint:     {zone['hint']}")
    else:
        cardgame_view.log("Visual CI: [PASS] All critical zones verified.")

    # 5. Asset presence check (heuristic)
    # Check if hand_self has cards (non-empty rect check isn't enough, 
    # but we can check if OCR sees anything in the status line)
    status = ocr_verify.read_zone("status_line")
    if status.get("ok"):
        cardgame_view.log(f"Visual CI: Status Line: '{status['text']}'")

    if not success:
        cardgame_view.log("Visual CI: Exiting with error code 1")
        return False
    
    cardgame_view.log("Visual CI: UI Verification Complete.")
    return True

if __name__ == "__main__":
    if not verify_hearts_ui():
        sys.exit(1)
