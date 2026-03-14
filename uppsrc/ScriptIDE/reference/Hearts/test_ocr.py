import ocr_verify
import cardgame_view

def test():
    cardgame_view.log("Running OCR diagnostic...")
    
    # 1. Check frame info
    info = ocr_verify.capture_frame()
    if info.get("ok"):
        cardgame_view.log(f"Capture OK: {info['width']}x{info['height']}")
    else:
        cardgame_view.log(f"Capture Failed: {info.get('error')}")
        return

    # 2. Read specific zone
    zone_id = "label_self"
    cardgame_view.log(f"Reading zone: {zone_id}...")
    res = ocr_verify.read_zone(zone_id)
    if res.get("ok"):
        cardgame_view.log(f"Zone {zone_id}: '{res['text']}' (conf: {res['confidence']:.2f})")
    else:
        cardgame_view.log(f"Zone {zone_id} failed: {res.get('error')}")

    # 3. Full compare
    cardgame_view.log("Running full visual comparison...")
    expected = {
        "label_self": "You  T:0  R:+0  C:13",
        "label_left": "West  T:0  R:+0  C:13",
        "label_top": "North  T:0  R:+0  C:13",
        "label_right": "East  T:0  R:+0  C:13"
    }
    report = ocr_verify.compare(expected)
    
    cardgame_view.log(f"Report Signal: {report['signal']}")
    cardgame_view.log(f"Zones Matched: {report['compare_matched']}/{report['compare_total']}")
    
    for zone in report['zones']:
        zid = zone['zone_id']
        if zone['match']:
            cardgame_view.log(f"  [PASS] {zid}")
        else:
            cardgame_view.log(f"  [FAIL] {zid}: expected '{zone.get('expected')}', got '{zone.get('observed')}'")
            cardgame_view.log(f"         hint: {zone.get('hint')}")

if __name__ == "__main__":
    test()
