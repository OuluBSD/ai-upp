# Test: Function after while loop with multi-line if

def fn_with_while_and_multiline_if():
    while True:
        if (x == 1
            and y == 2):
            do_something()
        break_time(10)

def fn_after_while():
    pass

print("Test passed!")
