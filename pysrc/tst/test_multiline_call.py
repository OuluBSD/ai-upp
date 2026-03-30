# Test: Multi-line function call in while loop

def test_fn():
    while True:
        if x == 1:
            do_something(
                2,
                callback_fn,
                None
            )
        break_time(10)

def next_fn():
    pass

print("Test passed!")
