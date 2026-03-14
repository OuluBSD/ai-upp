import ocr_verify
import cardgame_view
import time

def run_agent_loop(iterations=5):
    cardgame_view.log(f"Starting autonomous feedback loop ({iterations} iterations)...")
    
    # 1. Setup OCR models (one-time)
    ocr_verify.set_models("splitter.ocrmodel", "classifier.ocrmodel")
    
    for i in range(iterations):
        cardgame_view.log(f"\n--- Iteration {i+1} ---")
        
        # 2. Perform game action (represented here as a simple refresh)
        # In a real agent, this would be an AI-driven move or state change.
        cardgame_view.log("Agent performing action: Checking initial state...")
        
        # 3. Define visual expectations based on current logical state
        # In Hearts, we expect the initial player names and score 0.
        expected = {
            "label_self": "You  T:0  R:+0  C:13",
            "label_left": "West  T:0  R:+0  C:13",
            "label_top": "North  T:0  R:+0  C:13",
            "label_right": "East  T:0  R:+0  C:13"
        }
        
        # 4. Invoke OCR verification
        report = ocr_verify.compare(expected)
        
        # 5. Consume feedback
        if report["pass"]:
            cardgame_view.log("Visual Verification: [PASS]")
            cardgame_view.log(f"Signal: {report['signal']} (Matched: {report['compare_matched']}/{report['compare_total']})")
            # Proceed to next game logic step
        else:
            cardgame_view.log(f"Visual Verification: [{report['signal'].upper()}]")
            cardgame_view.log(f"Matched: {report['compare_matched']}/{report['compare_total']}")
            
            # 6. Analyze failures for corrective action
            for zone in report["zones"]:
                if not zone["match"]:
                    cardgame_view.log(f"  FAILED: {zone['zone_id']}")
                    cardgame_view.log(f"  Observed: '{zone['observed']}'")
                    cardgame_view.log(f"  Hint: {zone['hint']}")
            
            # In a real training loop, this feedback would be fed into the agent's 
            # prompt or reward function to encourage corrective generation.
            cardgame_view.log("Remediation triggered: self-correcting visual output...")
        
        time.sleep(1)

if __name__ == "__main__":
    run_agent_loop()
