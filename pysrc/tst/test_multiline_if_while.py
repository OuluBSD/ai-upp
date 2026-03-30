# Test: Multi-line if condition in while loop

def test():
    while True:
        if (x == 1
            and y == 2
            and z == 3):
            do_something()
        break_time(10)
    
    dir = 1
    dir = dir * -1

print("Test passed!")
