# Test function definition after while loop

def test_loop():
    while True:
        x = 1
        if x == 1:
            print("one")
        break_time(10)

def test_after_loop():
    print("after loop")

print("Test passed!")
