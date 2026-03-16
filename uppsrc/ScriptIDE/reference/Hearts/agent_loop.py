import ocr_verify
import cardgame_view
import time
import main

def run_agent_loop(iterations=10):
    cardgame_view.log(f"Starting interactive agent loop ({iterations} iterations)...")
    
    # 1. Setup OCR models (one-time)
    ocr_verify.set_models("splitter.ocrmodel", "classifier.ocrmodel")
    
    # Ensure the main simulation is started
    if not main.started:
        main.start()

    for i in range(iterations):
        cardgame_view.log(f"\n--- Iteration {i+1} ---")
        
        # Step 1: Check if we are in the PASSING phase
        if main.state.phase == 'PASSING':
            # Identify 3 cards in self hand to select
            hand_self = main.state.players[0]
            if len(hand_self) >= 13 and len(main.selected_cards) < 3:
                card_ids = [c.id for c in hand_self[:3]]
                cardgame_view.log(f"Agent Action: Selecting 3 cards {card_ids}")
                
                # Simulate selecting cards
                for cid in card_ids:
                    main.on_click(cid)
                
                # Verify that 3 cards are selected
                if len(main.selected_cards) == 3:
                    cardgame_view.log("Verification: 3 cards logically selected.")
                    
                    # Step 2: Verify visual state before passing
                    # (In a real test we'd check if they are highlighted)
                    
                    # Step 3: Trigger Pass Button
                    cardgame_view.log("Agent Action: Clicking 'button_pass'")
                    main.on_button("button_pass")
                    
                    # Step 4: Verify logically that cards are gone
                    new_hand_ids = [c.id for c in main.state.players[0]]
                    if all(cid not in new_hand_ids for cid in card_ids):
                        cardgame_view.log("Verification: PASS [Cards logically moved]")
                    else:
                        cardgame_view.log(f"Verification: FAIL [Cards still in hand: {new_hand_ids}]")
            else:
                cardgame_view.log(f"Agent Status: Phase is {main.state.phase}, hand size {len(hand_self)}, selected {len(main.selected_cards)}")

        # 2. Define visual expectations based on current logical state
        # Dynamically build expectations from main.state
        expected = {
            "label_self": f"You  T:{main.state.scores[0]}  R:+{main.state.round_scores[0]}  C:{len(main.state.players[0])}",
            "label_left": f"West  T:{main.state.scores[1]}  R:+{main.state.round_scores[1]}  C:{len(main.state.players[1])}",
            "label_top": f"North  T:{main.state.scores[2]}  R:+{main.state.round_scores[2]}  C:{len(main.state.players[2])}",
            "label_right": f"East  T:{main.state.scores[3]}  R:+{main.state.round_scores[3]}  C:{len(main.state.players[3])}"
        }
        
        # 3. Invoke OCR verification
        report = ocr_verify.compare(expected)
        
        # 4. Consume feedback
        if report["pass"]:
            cardgame_view.log(f"Visual Verification: [PASS] (Signal: {report['signal']} Match: {report['compare_matched']}/{report['compare_total']})")
        else:
            cardgame_view.log(f"Visual Verification: [{report['signal'].upper()}] Matched: {report['compare_matched']}/{report['compare_total']}")
            for zone in report["zones"]:
                if not zone["match"]:
                    cardgame_view.log(f"  FAILED: {zone['zone_id']} Expected: '{zone['expected']}' Observed: '{zone['observed']}'")
        
        time.sleep(1)

if __name__ == "__main__":
    run_agent_loop()
