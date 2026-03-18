import ocr_verify
import cardgame_view
import time
import sys
import main

def run_agent_loop():
    iterations = 20
    # Use a persistent list for our own tracking to avoid toggle issues
    our_selected_ids = []

    cardgame_view.log("Starting Hearts OCR Play Demo...")
    
    # 1. Setup OCR models (using the newly trained hearts model)
    ocr_verify.set_models("", "/common/active/sblo/Dev/PKR/bin/data/convnet_models/hearts_cards.model.bin")
    
    # Ensure the game simulation is started
    if not main.started:
        main.start()
        cardgame_view.log("Game simulation started.")

    for i in range(iterations):
        # 2. Vision Step: Read cards from our hand
        cardgame_view.log("Vision Action: Identifying cards in 'hand_self'...")
        observed_cards = ocr_verify.read_cards("hand_self")
        # ocr_verify.read_cards returns a list of labels
        cardgame_view.log("Vision Observed: " + " ".join(observed_cards))
        
        # 3. Strategy Step (Simplified for demo)
        if main.state.phase == 'PASSING':
            # Synchronize our_selected_ids with main.selected_cards (if main was reloaded)
            current_main_ids = [sc.id for sc in main.selected_cards]
            for sid in our_selected_ids[:]:
                if sid not in current_main_ids:
                    # main.selected_cards was likely cleared or reset
                    our_selected_ids.remove(sid)

            if len(main.selected_cards) < 3:
                hand_self = main.state.players[0]
                for card in hand_self:
                    if card.id in observed_cards and card.id not in our_selected_ids:
                        cardgame_view.log("Agent Action: Selecting " + card.id)
                        main.on_click(card.id)
                        our_selected_ids.append(card.id)
                        time.sleep(0.2) # settle time
                        break
            
            if len(main.selected_cards) == 3:
                cardgame_view.log("Agent Action: 3 cards selected, clicking 'button_pass'")
                main.on_button("button_pass")
                our_selected_ids.clear()
        # 4. Status Check
        expected = {
            "label_self": "You  T:" + str(main.state.scores[0]) + "  R:+" + str(main.state.round_scores[0]) + "  C:" + str(len(main.state.players[0]))
        }
        report = ocr_verify.compare(expected)
        if report["pass"]:
            cardgame_view.log("Visual Status: [OK]")
        else:
            cardgame_view.log("Visual Status: [MISMATCH]")
            
        time.sleep(1)

if __name__ == "__main__":
    run_agent_loop()
