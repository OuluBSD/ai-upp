# Test: Multi-line if with dict access and function call

def test_fn():
    while True:
        if (proximity(actor, door) < 40
            and not obj["alerting"]
            and obj["in_room"] == actor["in_room"]):
            obj["alerting"] = True
            obj["stopped_player"] = True
            cutscene(
                2,
                callback_fn,
                None
            )
        break_time(10)

def next_fn():
    pass

print("Test passed!")
