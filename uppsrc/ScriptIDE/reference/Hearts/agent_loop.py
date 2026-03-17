import ocr_verify
import cardgame_view
import time

def run_agent_loop():
    iterations = 5
    cardgame_view.log("Starting interactive agent loop...")
    
    # 1. Setup OCR models (one-time)
    ocr_verify.set_models("splitter.ocrmodel", "classifier.ocrmodel")
    ocr_verify.set_dataset_collection(True, "tmp/ocr_dataset")
    
    for i in range(iterations):
        cardgame_view.log("--- Iteration ---")
        
        # 2. Extract card patches
        cardgame_view.log("Agent Action: Extracting card patches from hand_self...")
        ocr_verify.read_cards("hand_self")
        
        # 3. Define visual expectations based on current logical state
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
        else:
            cardgame_view.log("Visual Verification: [FAIL]")
        
        time.sleep(1)

if __name__ == "__main__":
    run_agent_loop()
