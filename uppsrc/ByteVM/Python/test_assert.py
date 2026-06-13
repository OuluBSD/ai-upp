# Regression for Python assert handling and dict membership in ByteVM.

hand_state = {
    "board": {
        "count": 1,
        "queen": 2,
    }
}

assert "count" in hand_state["board"]
assert "joker" in hand_state["board"], "board missing key"
